#pragma once
#include <assert.h>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <lexer/position.hpp>
#include <lexer/token.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sysexits.h>

namespace lexer
{

class Exception : public std::runtime_error {
  public:
  Exception(const std::string& what)
    : std::runtime_error(what)
  { }
};

class Scanner {
  public:
  Scanner(const std::filesystem::path& filename)
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

  Scanner(const std::string& src)
  {
    contents = src;
    current = row = contents.c_str();
    line = 1;
  }

  std::string filename()
  {
    return filename_;
  }

  Token next()
  {
    skip_whitespaces();
    skip_comments();
    return read_token();
  }

  private:
  Token read_token();
  Token identifier();
  Token string_literal();
  Token integer_literal();
  Token punctuation();
  void skip_multiline_comment();
  std::string escape_sequence();

  char peek(int offset = 0)
  {
    if(current + offset >= contents.c_str() + contents.size())
    {
      return '\0';
    }
    return *(current + offset);
  }

  void expect(char ch, const std::string& err_msg)
  {
    if(*current != ch)
    {
      error_at(err_msg);
    }
    current++;
  }

  bool match(char ch)
  {
    if(*current == ch)
    {
      current++;
      return true;
    }
    return false;
  }

  void error_at(const std::string& err_msg)
  {
    error(std::format("[{}:{}] Err: {}", filename_, line, err_msg));
  }

  void error(const std::string& err_msg)
  {
    throw Exception(err_msg);
  }

  Token eof_token()
  {
    return Token(TokenType::eof, "$", Position(line, int(current - row) + 1));
  }

  bool is_eof(const char* current)
  {
    return current >= (contents.c_str() + contents.size());
  }

  void skip_comments()
  {
    while(!is_eof(current) && *current == '/' && peek(1) == '*')
    {
      skip_multiline_comment();
    }
  }

  void skip_whitespaces()
  {
    while(!is_eof(current) && std::isspace(*current))
    {
      if(*current == '\n')
      {
        line++;
        row = current + 1;
      }
      current++;
    }
  }

  private:
  std::string filename_;
  std::string contents;
  int line;
  const char* row;
  const char* current;
};

} // namespace lexer