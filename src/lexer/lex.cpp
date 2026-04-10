#include "lexer/lex.hpp"
#include "lexer/position.hpp"
#include <format>
#include <fstream>
#include <utility>

namespace lexer
{

Scanner::Scanner(const std::filesystem::path& filename)
  : filename_(filename)
{
  auto f = std::ifstream(filename);

  if(!f.is_open())
  {
    error(std::format("Err: could not open file '{}'", filename.generic_string()));
  }

  std::stringstream buffer;
  buffer << f.rdbuf();
  contents = buffer.str();
  current = row = contents.c_str();
  line = 1;
}

Scanner::Scanner(std::string src)
  : contents(std::move(src))
  , line(1)
  , current(row = contents.c_str())
{ }

auto Scanner::filename() -> std::string
{
  return filename_;
}

auto Scanner::next() -> Token
{
  skip_whitespaces();
  skip_comments();
  return read_token();
}

auto Scanner::peek(int offset) -> char
{
  if(current + offset >= contents.c_str() + contents.size())
  {
    return '\0';
  }
  return *(current + offset);
}

void Scanner::expect(char ch, const std::string& err_msg)
{
  if(*current != ch)
  {
    error_at(err_msg);
  }
  current++;
}

auto Scanner::match(char ch) -> bool
{
  if(*current == ch)
  {
    current++;
    return true;
  }
  return false;
}

void Scanner::error_at(const std::string& err_msg)
{
  error(std::format("[{}:{}] Err: {}", filename_, line, err_msg));
}

void Scanner::error(const std::string& err_msg)
{
  throw Exception(err_msg);
}

auto Scanner::eof_token() -> Token
{
  return {TokenType::eof, "$", Position(line, int(current - row) + 1)};
}

auto Scanner::is_eof(const char* current) -> bool
{
  return current >= (contents.c_str() + contents.size());
}

void Scanner::skip_comments()
{
  while(!is_eof(current) && *current == '/' && peek(1) == '*')
  {
    skip_multiline_comment();
  }
}

void Scanner::skip_whitespaces()
{
  while(!is_eof(current) && (std::isspace(*current) != 0))
  {
    if(*current == '\n')
    {
      line++;
      row = current + 1;
    }
    current++;
  }
}

auto Scanner::escape_sequence() -> std::string
{
  // current points to the backslash
  const std::string s(1, *current);

  const char ch1 = peek(1);
  const char ch2 = peek(2);
  const char ch3 = peek(3);
  // 3-digit octal
  if((std::isdigit(ch1) != 0) && (std::isdigit(ch2) != 0) && (std::isdigit(ch3) != 0))
  {
    int v = 0;
    v = ((ch1 - '0') * 64) + ((ch2 - '0') * 8) + (ch3 - '0');
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

auto Scanner::punctuation() -> Token
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
    if(match('>'))
    {
      return Token{TokenType::arrow, "->", Position(line, int(current - row) - 1)};
    }
    else
    {
      return Token{TokenType::minus_op, "-", Position(line, int(current - row))};
    }
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

auto Scanner::integer_literal() -> Token
{
  const char* start = current;
  while(!is_eof(current) && (std::isdigit(*current) != 0))
  {
    current++;
  }
  return {
    TokenType::integer_literal, std::string(start, current), Position(line, int(start - row) + 1)};
}

auto Scanner::string_literal() -> Token
{
  /* This is a bit different from the book. Multiline strings are like this:
  "hello\
  world" 
  We don't support escaping control characters like ^c. 
  */

  // first token is the opening quote
  current++; // skip it
  std::string value;
  const int sline = line;
  const int spos = int(current - row);
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

auto Scanner::identifier() -> Token
{

  const char* start = current;
  while(!is_eof(current) && ((std::isalnum(*current) != 0) || *current == '_'))
  {
    current++;
  }

  const std::string value(start, current);
  if(keywords.contains(value))
  {
    return {keywords.at(value), value, Position(line, int(start - row) + 1)};
  }
  return {TokenType::identifier, value, Position(line, int(start - row) + 1)};
}

auto Scanner::read_token() -> Token
{
  if(is_eof(current))
  {
    return eof_token();
  }
  else if(std::isalpha(*current) != 0)
  {
    return identifier();
  }
  else if(std::isdigit(*current) != 0)
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