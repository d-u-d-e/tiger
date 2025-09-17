#include <algorithm>
#include <codegen/arch/frame.hpp>
#include <codegen/assem.hpp>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <iostream>
#include <ir/fragment.hpp>
#include <ir/tree.hpp>
#include <iterator>
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
#include <utility>

#include <codegen/arch/isel.hpp>
#include <flow.hpp>
#include <ir/canon.hpp>
#include <liveness.hpp>
#include <variant>
#include <vector>

void code_gen(ir::tree::Stmt&& stmt, arch::Frame& f)
{
  auto sep = "-----------------------------";
  ir::tree::PrettyPrinter ir_pretty_printer;
  (void)ir_pretty_printer;

  /*
  std::cout << "IR" << "\n";
  std::cout << std::visit(ir_pretty_printer, stmt) << "\n" << sep << "\n";
  */

  ir::tree::Canon canon;
  auto list = canon.linearize(std::move(stmt));

  /*
  std::cout << "Reduced" << "\n";
  for(auto& s : list) {
    std::string reduced = std::visit(ir_pretty_printer, s);
    std::cout << reduced << "\n";
  }
  std::cout << sep << "\n";
  */

  auto [blocks, ldone] = canon.basic_blocks(std::move(list));

  /*
  std::cout << "Basic blocks" << "\n";
  for(auto& b : blocks) {
    std::cout << "<<<< block start" << "\n";
    for(auto& s : b.stmts) {
      std::string irstr = std::visit(ir_pretty_printer, s);
      std::cout << irstr << "\n";
    }
    std::cout << ">>>> block end" << "\n" << "\n";
  }
  std::cout << sep << "\n";
  */

  auto sched = canon.trace_schedule(std::move(blocks), ldone);

  /* 
  std::cout << "Trace" << "\n";
  for(auto& s : sched) {
    std::string irstr = std::visit(ir_pretty_printer, s);
    std::cout << irstr << "\n";
  }
  std::cout << sep << "\n";
  */

  arch::codegen::MuxMunchGen gen;
  std::vector<::codegen::assem::Instruction> all;
  for(auto& s : sched)
  {
    auto v = gen.gen(s);
    std::move(v.begin(), v.end(), std::back_inserter(all));
  }

  auto print_instr = [](const std::vector<::codegen::assem::Instruction>& instrs) {
    for(auto& i : instrs)
    {
      std::cout << arch::codegen::format(arch::Frame::map_temp, i);
    }
  };

  f.proc_entry_exit2(all);
  auto [pro, epi] = f.proc_entry_exit3(all);
  std::cout << pro;
  print_instr(all);
  std::cout << epi;
  std::cout << sep << "\n";

  // Create the control flow graph
  auto flow_g = flow::FlowGraph(all);
  flow_g.render(f.name().str(), f.name().str());

  liveness::LivenessAnalyzer analyzer(flow_g);
  std::cout << analyzer.dump_result() << "\n";
  std::cout << sep << "\n";
}

int main(int argc, char** argv)
{
  if(argc != 2)
  {
    std::cerr << "\033[1;31m";
    std::cerr << "tigerc: no input files\n";
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
  std::cout << string_table.dump() << "\n";
  */

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
    std::cerr << "\033[1;31m" << e.what() << "\033[0m" << "\n";
    return EX_DATAERR;
  }

  translator.translate_main_program(std::move(ir));

  // dump procedure fragments
  for(auto& frag : translator.fragments())
  {
    if(std::holds_alternative<ir::ProcedureFragment>(frag))
    {
      auto& pf = std::get<ir::ProcedureFragment>(frag);
      code_gen(std::move(pf.body), *pf.level->frame);
    }
  }

  // print output of codegen
  return EX_OK;
}