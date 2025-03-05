#include <iostream>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

#include <semantic/env.hpp>
#include <semantic/type_checker.hpp>
#include <symbol.hpp>

int main(int argc, char** argv)
{
  std::filesystem::path s = argv[1];

  lexer::Scanner scanner(s);
  lexer::TokenType type;

  symbol::StringTable string_table;
  parser::Parser parser(scanner, string_table);
  auto exp = parser.parse();

  // pretty print the ast
  parser::ast::PrettyPrinter pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl << std::endl;

  // dump the string table
  std::cout << "string table:" << std::endl;
  std::cout << string_table.dump() << std::endl;

  auto tenv =
    std::make_shared<semantic::env::Environment<semantic::env::TEntry>>();
  auto venv =
    std::make_shared<semantic::env::Environment<semantic::env::VEntry>>();

  semantic::TypeChecker type_checker(tenv, venv);
}