#include <lexer/token.hpp>

namespace lexer
{

// clang-format off
const std::unordered_map<std::string, TokenType> keywords = {
    {"while", TokenType::while_keyword},
    {"for", TokenType::for_keyword},
    {"to", TokenType::to_keyword},
    {"break", TokenType::break_keyword},
    {"let", TokenType::let_keyword},
    {"in", TokenType::in_keyword},
    {"end", TokenType::end_keyword},
    {"function", TokenType::function_keyword},
    {"var", TokenType::var_keyword},
    {"type", TokenType::type_keyword},
    {"array", TokenType::array_keyword},
    {"if", TokenType::if_keyword},
    {"then", TokenType::then_keyword},
    {"else", TokenType::else_keyword},
    {"do", TokenType::do_keyword},
    {"of", TokenType::of_keyword},
    {"nil", TokenType::nil_keyword},
};
// clang-format on

} // namespace lexer