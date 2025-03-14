#include <iostream>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

#include <semantic/analyzer.hpp>
#include <semantic/env.hpp>
#include <symbol.hpp>
#include <sysexits.h>

// TODO
#include <translation/ir.hpp>

int main(int argc, char** argv)
{
  std::filesystem::path s = argv[1];

  lexer::Scanner scanner(s);
  lexer::TokenType type;

  symbol::StringTable string_table;
  parser::Parser parser(std::cerr, scanner, string_table);

  std::cerr << "\033[1;31m";
  auto exp = parser.parse();
  if(parser.had_error()) {
    return EX_DATAERR;
  }
  std::cerr << "\033[0m";

  // pretty print the ast
  parser::ast::PrettyPrinter pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl << std::endl;

  // dump the string table
  std::cout << "string table:" << std::endl;
  std::cout << string_table.dump() << std::endl;

  translation::ir::Translator translator;
  semantic::Analyzer type_checker(string_table, translator);
  type_checker.type_check(*exp);
}