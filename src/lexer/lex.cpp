#include <cctype>
#include <format>
#include <lexer/lex.hpp>
#include <lexer/position.hpp>
#include <lexer/token.hpp>
#include <string>
#include <utility>

namespace lexer
{

std::string Scanner::escape_sequence()
{
  // current points to the backslash
  std::string s(1, *current);

  char ch1 = peek(1);
  char ch2 = peek(2);
  char ch3 = peek(3);
  // 3-digit octal
  if(std::isdigit(ch1) && std::isdigit(ch2) && std::isdigit(ch3))
  {
    int v = 0;
    v = (ch1 - '0') * 64 + (ch2 - '0') * 8 + (ch3 - '0');
    if(v > 255)
    {
      error_at("3-digit octal escape sequence out of range");
    }
    return s + ch1 + ch2 + ch3;
  }

  if(ch1 == '"' || ch1 == '\\' || ch1 == 'n' || ch1 == 't')
  {
    return s + ch1;
  }
  error_at(std::format("invalid escape sequence '\\{}'", ch1));
  std::unreachable();
}

Token Scanner::punctuation()
{
  switch(*current)
  {
  case ',':
    current++;
    return Token{TokenType::comma, ",", Position(line, int(current - row))};
  case ':':
    current++;
    if(match('='))
    {
      return Token{TokenType::assign_op, ":=", Position(line, int(current - row) - 1)};
    }
    return Token{TokenType::colon, ":", Position(line, int(current - row))};
  case ';':
    current++;
    return Token{TokenType::semicolon, ";", Position(line, int(current - row))};
  case '(':
    current++;
    return Token{TokenType::lparen, "(", Position(line, int(current - row))};
  case ')':
    current++;
    return Token{TokenType::rparen, ")", Position(line, int(current - row))};
  case '{':
    current++;
    return Token{TokenType::lbrace, "{", Position(line, int(current - row))};
  case '}':
    current++;
    return Token{TokenType::rbrace, "}", Position(line, int(current - row))};
  case '.':
    current++;
    return Token{TokenType::dot_op, ".", Position(line, int(current - row))};
  case '+':
    current++;
    return Token{TokenType::plus_op, "+", Position(line, int(current - row))};
  case '-':
    current++;
    return Token{TokenType::minus_op, "-", Position(line, int(current - row))};
  case '*':
    current++;
    return Token{TokenType::times_op, "*", Position(line, int(current - row))};
  case '/':
    current++;
    return Token{TokenType::divide_op, "/", Position(line, int(current - row))};
  case '=':
    current++;
    return Token{TokenType::equal_op, "=", Position(line, int(current - row))};
  case '<':
    current++;
    if(match('='))
    {
      return Token{TokenType::less_equal_op, "<=", Position(line, int(current - row) - 1)};
    }
    else if(match('>'))
    {
      return Token{TokenType::not_equal_op, "<>", Position(line, int(current - row) - 1)};
    }
    else
    {
      return Token{TokenType::less_op, "<", Position(line, int(current - row))};
    }
  case '>':
    current++;
    if(match('='))
    {
      return Token{TokenType::greater_equal_op, ">=", Position(line, int(current - row) - 1)};
    }
    return Token{TokenType::greater_op, ">", Position(line, int(current - row))};
  case '&':
    current++;
    return Token{TokenType::and_op, "&", Position(line, int(current - row))};
  case '|':
    current++;
    return Token{TokenType::or_op, "|", Position(line, int(current - row))};
  case '[':
    current++;
    return Token{TokenType::lbracket, "[", Position(line, int(current - row))};
  case ']':
    current++;
    return Token{TokenType::rbracket, "]", Position(line, int(current - row))};
  default:
    error_at(std::format("invalid character '{}'", *current));
  }
  std::unreachable();
}

Token Scanner::integer_literal()
{
  const char* start = current;
  while(!is_eof(current) && std::isdigit(*current))
  {
    current++;
  }
  return {
    TokenType::integer_literal, std::string(start, current), Position(line, int(start - row) + 1)};
}

Token Scanner::string_literal()
{
  /* This is a bit different from the book. Multiline strings are like this:
  "hello\
  world" 
  We don't support escaping control characters like ^c. 
  */

  // first token is the opening quote
  current++; // skip it
  std::string value;
  int sline = line;
  int spos = int(current - row);
  while(!is_eof(current))
  {
    if(*current == '\\')
    {
      if(peek(1) == '\n')
      {
        current += 2;
        line++;
      }
      else
      {
        auto seq = escape_sequence();
        value += seq;
        current += seq.size();
      }
    }
    else if(*current == '\n' || *current == '\r')
    {
      error_at("unterminated string literal");
    }
    else if(*current == '"')
    {
      // closing quote
      current++; // skip it
      return Token{TokenType::string_literal, value, Position(sline, spos)};
    }
    else
    {
      value += *current;
      current++;
    }
  }

  error_at("unterminated string literal");
  std::unreachable();
}

Token Scanner::identifier()
{

  const char* start = current;
  while(!is_eof(current) && (std::isalnum(*current) || *current == '_'))
  {
    current++;
  }

  std::string value(start, current);
  if(keywords.find(value) != keywords.end())
  {
    return {keywords.at(value), value, Position(line, int(start - row) + 1)};
  }
  return {TokenType::identifier, value, Position(line, int(start - row) + 1)};
}

Token Scanner::read_token()
{
  if(is_eof(current))
  {
    return eof_token();
  }
  else if(std::isalpha(*current))
  {
    return identifier();
  }
  else if(std::isdigit(*current))
  {
    return integer_literal();
  }
  else if(*current == '"')
  {
    return string_literal();
  }
  return punctuation();
}

void Scanner::skip_multiline_comment()
{
  current += 2;
  while(!is_eof(current))
  {
    if(*current == '*' && peek(1) == '/')
    {
      current += 2;
      skip_whitespaces();
      return;
    }
    if(*current == '\n')
    {
      line++;
    }
    current++;
  }
  error_at("unterminated multiline comment");
}

} // namespace lexer