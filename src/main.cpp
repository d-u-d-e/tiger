#include <exception>
#include <filesystem>
#include <iostream>
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
#include <utility>

#include <ir/canon.hpp>

int main(int argc, char** argv)
{
  if(argc != 2) {
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

  if(parser.had_error()) {
    return EX_DATAERR;
  }

  /*
  // pretty print the ast
  parser::ast::PrettyPrinter pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl << std::endl;

  // dump the string table
  std::cout << "string table:" << std::endl;
  std::cout << string_table.dump() << std::endl;*/

  // find escape variables
  seman::EscapeFinder esc_finder;
  exp->accept(esc_finder);

  ir::Translator translator;
  seman::Analyzer type_checker(string_table, translator);
  ir::Exp ir;
  try {
    ir = type_checker.type_check(*exp);
  }
  catch(std::exception& e) {
    std::cerr << "\033[1;31m" << e.what() << "\033[0m" << std::endl;
    return EX_DATAERR;
  }

  ir::tree::PrettyPrinter ir_pretty_printer;
  auto c = translator.unex(std::move(ir));

  std::string r = std::visit(ir_pretty_printer, c);
  std::cout << r << std::endl << std::endl;

  for(auto& frag : translator.fragments()) {
    std::cout << translator.dump_fragment(frag) << std::endl;
  }

  return EX_OK;
}