#include <iostream>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

#include <semantic/env.hpp>
#include <symbol.hpp>

#include <symbol_table.hpp>

int main(int argc, char** argv)
{
  std::filesystem::path s = argv[1];

  lexer::Scanner scanner(s);
  lexer::TokenType type;

  symbol::StringTable string_table;
  parser::Parser parser(scanner, string_table);
  auto exp = parser.parse();

  // pretty print the ast
  ASTVisitor pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl << std::endl;

  // dump the string table
  std::cout << "string table:" << std::endl;
  std::cout << string_table.dump() << std::endl;

  // create the environments for the type checker
  using namespace semantic::environment;
  Environment<VEntry> venv;
  Environment<TEntry> tenv;

  symbol::SymbolTable<TEntry> t;

  /*
    let
      type T := int
      let 
        type T := string
       in
      end
      type R := {x: int, y: string, z: string}
      type A := array of string
    in
    end
  */

  auto T = symbol::Symbol("T", 1);
  auto x = symbol::Symbol("x", 2);
  auto y = symbol::Symbol("y", 3);
  auto z = symbol::Symbol("z", 4);
  auto R = symbol::Symbol("R", 5);
  auto A = symbol::Symbol("A", 6);

  tenv.begin_scope();
  tenv.enter(T, std::make_shared<semantic::types::Integer>());

  tenv.begin_scope();
  tenv.enter(T, std::make_shared<semantic::types::String>());

  std::cout << tenv.dump() << "\n\n";
  tenv.end_scope();

  std::vector<std::pair<symbol::Symbol, std::shared_ptr<semantic::types::Type>>>
    fields;
  fields.emplace_back(x, std::make_shared<semantic::types::Integer>());
  fields.emplace_back(y, std::make_shared<semantic::types::String>());
  fields.emplace_back(z, std::make_shared<semantic::types::String>());
  tenv.enter(R, std::make_shared<semantic::types::Record>(fields, 0));

  tenv.enter(A,
             std::make_shared<semantic::types::Array>(
               std::make_shared<semantic::types::String>(), 1));

  std::cout << tenv.dump() << "\n";
  // lookup R
  auto type_R = tenv.lookup(R).value();
  std::cout << "lookup R: " << type_R->to_string() << "\n\n";
  tenv.end_scope();
  std::cout << tenv.dump() << "\n\n";

  t.enter(T, std::make_shared<semantic::types::String>());
  auto e = t.lookup(T);
  auto entry = e.value()->to_string();

}