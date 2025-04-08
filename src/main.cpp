#include <codegen/arch/frame.hpp>
#include <exception>
#include <filesystem>
#include <iostream>
#include <ir/fragment.hpp>
#include <ir/tree.hpp>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

#include <seman/analyzer.hpp>
#include <seman/env.hpp>
#include <symbol.hpp>
#include <sysexits.h>

#include <ir/pretty_printer.hpp>
#include <ir/translator.hpp>
#include <seman/escape.hpp>
#include <string>
#include <utility>

#include <codegen/arch/isel.hpp>
#include <ir/canon.hpp>
#include <variant>

int main(int argc, char** argv)
{
  if(argc != 2)
  {
    std::cerr << "\033[1;31m";
    std::cerr << "tiger: no input files";
    std::cerr << "\033[0m";
    return EX_NOINPUT;
  };

  std::filesystem::path s = argv[1];

  lexer::Scanner scanner(s);
  symbol::StringTable string_table;
  parser::Parser parser(std::cerr, scanner, string_table);

  std::cerr << "\033[1;31m";
  auto exp = parser.parse();
  std::cerr << "\033[0m";

  if(parser.had_error())
  {
    return EX_DATAERR;
  }

  /*
  // pretty print the ast
  parser::ast::PrettyPrinter pretty_printer;
  std::cout << exp->accept(pretty_printer) << "\n" << "\n";

  // dump the string table
  std::cout << "string table:" << "\n";
  std::cout << string_table.dump() << "\n";*/

  // find escape variables
  seman::EscapeFinder esc_finder;
  exp->accept(esc_finder);

  ir::Translator translator;
  seman::Analyzer type_checker(string_table, translator);
  ir::Exp ir;
  try
  {
    ir = type_checker.type_check(*exp);
  }
  catch(std::exception& e)
  {
    std::cerr << "\033[1;31m" << e.what() << "\033[0m"
              << "\n";
    return EX_DATAERR;
  }

  ir::tree::PrettyPrinter ir_pretty_printer;
  auto c = translator.unex(std::move(ir));

  auto sep = "-----------------------------";
  std::cout << "IR: main expression"
            << "\n";
  std::cout << std::visit(ir_pretty_printer, c) << "\n" << sep << "\n";
  ir::tree::Canon canon;

  // dump procedure fragments
  // procedure fragments are shown before and after canonicalization
  for(auto& frag : translator.fragments())
  {
    if(std::holds_alternative<ir::ProcedureFragment>(frag))
    {
      auto& pf = std::get<ir::ProcedureFragment>(frag);

      std::cout << "IR: proc fragment"
                << "\n";
      std::cout << translator.dump_fragment(frag) << "\n" << sep << "\n";

      auto list = canon.linearize(std::move(pf.body));

      /*std::cout << "IR: proc fragment reduced" << "\n";
      for(auto& s : list) {
        std::string reduced = std::visit(ir_pretty_printer, s);
        std::cout << reduced << "\n";
      }
      std::cout << sep << "\n";*/

      auto [blocks, ldone] = canon.basic_blocks(std::move(list));

      /*std::cout << "IR: proc fragment basic blocks" << "\n";
      for(auto& b : blocks) {
        std::cout << "<<<< block start" << "\n";
        for(auto& s : b.stmts) {
          std::string irstr = std::visit(ir_pretty_printer, s);
          std::cout << irstr << "\n";
        }
        std::cout << ">>>> block end" << "\n" << "\n";
      }
      std::cout << sep << "\n";*/

      // print the traces
      auto sched = canon.trace_schedule(std::move(blocks), ldone);
      std::cout << "IR: trace"
                << "\n";
      for(auto& s : sched)
      {
        std::string irstr = std::visit(ir_pretty_printer, s);
        std::cout << irstr << "\n";
      }
      std::cout << sep << "\n";

      // print the asm without register allocation
      arch::codegen::MuxMunchGen gen;
      std::cout << "ASM: without reg alloc"
                << "\n";
      for(auto& s : sched)
      {
        auto instrs = gen.gen(s);
        for(auto& i : instrs)
        {
          std::cout << arch::codegen::format(arch::Frame::map_temp, i) << "\n";
        }
      }
      std::cout << sep << "\n";
    }
  }

  // print output of codegen
  return EX_OK;
}