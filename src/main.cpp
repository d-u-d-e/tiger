#include <iostream>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

#include <symbol.hpp>

int main(int argc, char** argv)
{
  std::filesystem::path s = argv[1];

  lexer::Scanner scanner(s);
  lexer::TokenType type;
  /*do {
    auto token = scanner.next();
    type = token.type;
    std::cout << token.value << ", " << lexer::to_string(type) << ", "
              << token.line << std::endl;
  } while(type != lexer::TokenType::eof);*/

  symbol::SymbolTable symbol_table;
  parser::Parser parser(scanner, symbol_table);
  auto exp = parser.parse();

  // do something with it
  ASTVisitor pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl << std::endl;

  // dump the symbol table
  std::cout << "Symbol Table:" << std::endl;
  std::cout << symbol_table.dump();
}
