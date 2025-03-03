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
  std::cout << symbol_table.dump() << std::endl;

  // create the environments for the type checker
  using namespace semantic::environment;
  Environment<VEntry> venv;
  Environment<TEntry> tenv;

  std::cout << "Environment:" << std::endl;
  tenv.insert(0, std::make_shared<semantic::types::Integer>());
  tenv.insert(0, std::make_shared<semantic::types::String>());

  std::vector<std::pair<symbol::Symbol, std::shared_ptr<semantic::types::Type>>>
    fields;
  fields.push_back(std::make_pair(
    symbol::Symbol("x", 0), std::make_shared<semantic::types::Integer>()));
  fields.push_back(std::make_pair(symbol::Symbol("y", 1),
                                  std::make_shared<semantic::types::String>()));
  fields.push_back(std::make_pair(symbol::Symbol("z", 2),
                                  std::make_shared<semantic::types::String>()));
  tenv.insert(1, std::make_shared<semantic::types::Record>(fields, 100));

  tenv.insert(1,
              std::make_shared<semantic::types::Array>(
                std::make_shared<semantic::types::String>(), 200));

  std::cout << tenv.dump() << std::endl;
}
