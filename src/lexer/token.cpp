#include "lexer/token.hpp"
#include <cassert>
#include <format>
#include <string>
#include <unordered_map>
#include <utility>

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

std::string to_string(TokenType type)
{
  switch(type)
  {
  case TokenType::eof:
    return "eof";
  case TokenType::identifier:
    return "identifier";
  case TokenType::integer_literal:
    return "integer_literal";
  case TokenType::string_literal:
    return "string_literal";
  case TokenType::while_keyword:
    return "while_keyword";
  case TokenType::for_keyword:
    return "for_keyword";
  case TokenType::to_keyword:
    return "to_keyword";
  case TokenType::break_keyword:
    return "break_keyword";
  case TokenType::let_keyword:
    return "let_keyword";
  case TokenType::in_keyword:
    return "in_keyword";
  case TokenType::end_keyword:
    return "end_keyword";
  case TokenType::function_keyword:
    return "function_keyword";
  case TokenType::var_keyword:
    return "var_keyword";
  case TokenType::type_keyword:
    return "type_keyword";
  case TokenType::array_keyword:
    return "array_keyword";
  case TokenType::if_keyword:
    return "if_keyword";
  case TokenType::then_keyword:
    return "then_keyword";
  case TokenType::else_keyword:
    return "else_keyword";
  case TokenType::do_keyword:
    return "do_keyword";
  case TokenType::of_keyword:
    return "of_keyword";
  case TokenType::nil_keyword:
    return "nil_keyword";
  case TokenType::comma:
    return "comma";
  case TokenType::colon:
    return "colon";
  case TokenType::semicolon:
    return "semicolon";
  case TokenType::lparen:
    return "lparen";
  case TokenType::rparen:
    return "rparen";
  case TokenType::lbracket:
    return "lbracket";
  case TokenType::rbracket:
    return "rbracket";
  case TokenType::lbrace:
    return "lbrace";
  case TokenType::rbrace:
    return "rbrace";
  case TokenType::dot_op:
    return "dot_op";
  case TokenType::plus_op:
    return "plus_op";
  case TokenType::minus_op:
    return "minus_op";
  case TokenType::times_op:
    return "times_op";
  case TokenType::divide_op:
    return "divide_op";
  case TokenType::equal_op:
    return "equal_op";
  case TokenType::not_equal_op:
    return "not_equal_op";
  case TokenType::less_op:
    return "less_op";
  case TokenType::less_equal_op:
    return "less_equal_op";
  case TokenType::greater_op:
    return "greater_op";
  case TokenType::greater_equal_op:
    return "greater_equal_op";
  case TokenType::and_op:
    return "and_op";
  case TokenType::or_op:
    return "or_op";
  case TokenType::assign_op:
    return "assign_op";
  }
  assert(false);
  std::unreachable();
}

std::string to_string(const Token& token)
{
  return std::format(
    "[{}: '{}' ({}, {})]", to_string(token.type), token.value, token.pos.line, token.pos.column);
}
} // namespace lexer