#include <lexer/lex.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    lexer::Scanner scanner(argv[1]);
    lexer::TokenType type;
    do {
        auto token = scanner.next();
        type = token.type;
        std::cout << token.value << ", " << lexer::to_string(type) << std::endl;
    } while (type != lexer::TokenType::eof);

}
