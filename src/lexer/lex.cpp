#include <lexer/lex.hpp>
#include <utility>

namespace lexer
{

char Scanner::escape_sequence(const char** current)
{
  // current points to the backslash
  char ch1 = peek(1);
  char ch2 = peek(2);
  char ch3 = peek(3);

  // 3-digit octal
  if(std::isdigit(ch1) && std::isdigit(ch2) && std::isdigit(ch3)) {
    int v = 0;
    v = (ch1 - '0') * 64 + (ch2 - '0') * 8 + (ch3 - '0');
    if(v > 255) {
      error("3-digit octal escape sequence out of range");
    }
    *current += 4;
    return (char)v;
  }
  switch(ch1) {
  case '"':
    *current += 2;
    return '"';
  case '\\':
    *current += 2;
    return '\\';
  case 'n':
    *current += 2;
    return '\n';
  case 't':
    *current += 2;
    return '\t';
  default:
    break;
  }
  error(std::format("Invalid escape sequence '\\{}'", ch1));
  std::unreachable();
}

Token Scanner::punctuation()
{
  switch(*current) {
  case ',':
    current++;
    return Token{TokenType::comma, ",", Position(line, int(current - row))};
  case ':':
    current++;
    if(match('=')) {
      return Token{
        TokenType::assign_op, ":=", Position(line, int(current - row) - 1)};
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
    if(match('=')) {
      return Token{
        TokenType::less_equal_op, "<=", Position(line, int(current - row) - 1)};
    }
    else if(match('>')) {
      return Token{
        TokenType::not_equal_op, "<>", Position(line, int(current - row) - 1)};
    }
    else {
      return Token{TokenType::less_op, "<", Position(line, int(current - row))};
    }
  case '>':
    current++;
    if(match('=')) {
      return Token{TokenType::greater_equal_op,
                   ">=",
                   Position(line, int(current - row) - 1)};
    }
    return Token{
      TokenType::greater_op, ">", Position(line, int(current - row))};
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
    break;
  }

  error(std::format("Invalid character {}!\n", *current));
  std::unreachable();
}

Token Scanner::integer_literal()
{
  const char* start = current;
  while(!is_eof(current) && std::isdigit(*current)) {
    current++;
  }
  return {TokenType::integer_literal,
          std::string(start, current),
          Position(line, int(current - row))};
}

Token Scanner::string_literal()
{
  /* This is a bit different from the book. Multiline strings are like this:
  "hello\
  world" 
  We don't support escaping control characters like ^c. 
  */

  current++; // first token is the opening quote
  std::string value;
  int sline = line;
  int spos = int(current - row);
  while(!is_eof(current)) {
    if(*current == '\\') {
      if(peek(1) == '\n') {
        current += 2;
        line++;
      }
      else {
        char ch = escape_sequence(&current);
        value += ch;
      }
    }
    else if(*current == '\n' || *current == '\r') {
      error("Unterminated string literal");
    }
    else if(*current != '"') {
      value += *current;
      current++;
    }
    else {
      current++; // closing quote
      return Token{TokenType::string_literal, value, Position(sline, spos)};
    }
  }

  error("Unterminated string literal");
  std::unreachable();
}

Token Scanner::identifier()
{

  const char* start = current;
  while(!is_eof(current) && (std::isalnum(*current) || *current == '_')) {
    current++;
  }

  std::string value(start, current);
  if(keywords.find(value) != keywords.end()) {
    return {keywords.at(value), value, Position(line, int(start - row) + 1)};
  }
  return {TokenType::identifier, value, Position(line, int(start - row) + 1)};
}

Token Scanner::read_token()
{
  if(!is_eof(current) && *current == '/' && peek(1) == '*') {
    skip_multiline_comment();
  }

  if(is_eof(current)) {
    return eof_token();
  }
  else if(std::isalpha(*current)) {
    return identifier();
  }
  else if(std::isdigit(*current)) {
    return integer_literal();
  }
  else if(*current == '"') {
    return string_literal();
  }
  return punctuation();
}

void Scanner::skip_multiline_comment()
{
  current += 2;
  while(!is_eof(current)) {
    if(*current == '*' && peek(1) == '/') {
      current += 2;
      skip_whitespaces();
      return;
    }
    if(*current == '\n') {
      line++;
    }
    current++;
  }
  error("Unterminated multiline comment");
}

} // namespace lexer