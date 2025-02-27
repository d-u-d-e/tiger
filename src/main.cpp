#include <iostream>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

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

  parser::Parser parser(scanner);
  auto exp = parser.parse();

  // do something with it
  ASTVisitor pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl;
}
