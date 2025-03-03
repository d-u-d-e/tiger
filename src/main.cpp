#include <iostream>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

#include <semantic/env.hpp>
#include <symbol.hpp>

int main(int argc, char** argv)
{
  std::filesystem::path s = argv[1];

  lexer::Scanner scanner(s);
  lexer::TokenType type;

  symbol::SymbolTable symbol_table;
  parser::Parser parser(scanner, symbol_table);
  auto exp = parser.parse();

  // pretty print the ast
  ASTVisitor pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl << std::endl;

  // dump the symbol table
  std::cout << "Symbol Table:" << std::endl;
  std::cout << symbol_table.dump();

  // create the environments for the type checker
  using namespace semantic::environment;
  Environment<Entry> venv;
  Environment<std::shared_ptr<semantic::types::Type>> tenv;
}
