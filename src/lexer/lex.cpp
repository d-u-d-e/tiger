#include <lexer/lex.hpp>

namespace lexer
{

char Scanner::escape_sequence(const char** current)
{
  // Current points to the backslash
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
  error("Invalid escape sequence");
  __builtin_unreachable();
}

Token Scanner::punctuation()
{
  switch(*current) {
  case ',':
    current++;
    return Token{TokenType::comma, ",", line};
  case ':':
    current++;
    if(match('=')) {
      return Token{TokenType::assign_op, ":=", line};
    }
    return Token{TokenType::colon, ":", line};
  case ';':
    current++;
    return Token{TokenType::semicolon, ";", line};
  case '(':
    current++;
    return Token{TokenType::lparen, "(", line};
  case ')':
    current++;
    return Token{TokenType::rparen, ")", line};
  case '{':
    current++;
    return Token{TokenType::lbrace, "{", line};
  case '}':
    current++;
    return Token{TokenType::rbrace, "}", line};
  case '.':
    current++;
    return Token{TokenType::dot_op, ".", line};
  case '+':
    current++;
    return Token{TokenType::plus_op, "+", line};
  case '-':
    current++;
    return Token{TokenType::minus_op, "-", line};
  case '*':
    current++;
    return Token{TokenType::times_op, "*", line};
  case '/':
    current++;
    return Token{TokenType::divide_op, "/", line};
  case '=':
    current++;
    return Token{TokenType::equal_op, "=", line};
  case '<':
    current++;
    if(match('=')) {
      return Token{TokenType::less_equal_op, "<=", line};
    }
    else if(match('>')) {
      return Token{TokenType::not_equal_op, "<>", line};
    }
    else {
      return Token{TokenType::less_op, "<", line};
    }
  case '>':
    current++;
    if(match('=')) {
      return Token{TokenType::greater_equal_op, ">=", line};
    }
    return Token{TokenType::greater_op, ">", line};
  case '&':
    current++;
    return Token{TokenType::and_op, "&", line};
  case '|':
    current++;
    return Token{TokenType::or_op, "|", line};
  case '[':
    current++;
    return Token{TokenType::lbracket, "[", line};
  case ']':
    current++;
    return Token{TokenType::rbracket, "]", line};
  default:
    break;
  }

  error(std::format("Invalid character {}!\n", *current));
  __builtin_unreachable();
}

Token Scanner::integer_literal()
{
  const char* start = current;
  while(!is_eof(current) && std::isdigit(*current)) {
    current++;
  }
  return {TokenType::integer_literal, std::string(start, current), line};
}

Token Scanner::string_literal()
{
  /* This is a bit different from the book. 
	Multiline strings are like this:
		"hello\
		world"
		We don't support escaping control characters like ^c. 
	*/

  current++; // First token is the opening quote
  std::string value;
  int sline = line;
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
      current++; // Closing quote
      return Token{TokenType::string_literal, value, sline};
    }
  }

  error("Unterminated string literal");
  __builtin_unreachable();
}

Token Scanner::identifier()
{

  const char* start = current;
  while(!is_eof(current) && (std::isalnum(*current) || *current == '_')) {
    current++;
  }

  std::string value(start, current);
  if(keywords.find(value) != keywords.end()) {
    return {keywords.at(value), value, line};
  }
  return {TokenType::identifier, value, line};
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