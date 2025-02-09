#include <iostream>
#include <lexer/lex.hpp>

int main(int argc, char** argv)
{
  std::string s = argv[1];
  lexer::Scanner scanner(s);
  lexer::TokenType type;
  do {
    auto token = scanner.next();
    type = token.type;
    std::cout << token.value << ", " << lexer::to_string(type) << ", "
              << token.line << std::endl;
  } while(type != lexer::TokenType::eof);
}
