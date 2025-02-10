#pragma once
#include <assert.h>
#include <format>
#include <ostream>
#include <string>
#include <unordered_map>

namespace lexer
{

enum class TokenType
{
  eof,
  identifier,
  integer_literal,
  string_literal,

  // keywords
  while_keyword,
  for_keyword,
  to_keyword,
  break_keyword,
  let_keyword,
  in_keyword,
  end_keyword,
  function_keyword,
  var_keyword,
  type_keyword,
  array_keyword,
  if_keyword,
  then_keyword,
  else_keyword,
  do_keyword,
  of_keyword,
  nil_keyword,

  // punctuation
  comma,
  colon,
  semicolon,
  lparen,
  rparen,
  lbracket,
  rbracket,
  lbrace,
  rbrace,
  dot_op,
  plus_op,
  minus_op,
  times_op,
  divide_op,
  equal_op,
  not_equal_op,
  less_op,
  less_equal_op,
  greater_op,
  greater_equal_op,
  and_op,
  or_op,
  assign_op,
};

struct Token {
  TokenType type;
  std::string value;
  int line;

  bool operator==(const Token& other) const
  {
    return type == other.type && value == other.value && line == other.line;
  }
};

std::string to_string(TokenType type);
std::string to_string(const Token& token);
std::ostream& operator<<(std::ostream& os, const Token& value);
extern const std::unordered_map<std::string, TokenType> keywords;

} // namespace lexer