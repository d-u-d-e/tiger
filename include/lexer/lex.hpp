#pragma once
#include "token.hpp"
#include <assert.h>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

namespace lexer
{

class Scanner {
  public:
  Scanner(const std::filesystem::path& filename)
  {
    auto f = std::ifstream(filename);

    if(!f.is_open()) {
      std::cerr << "Could not open file: " << filename << std::endl;
      exit(1);
    }

    std::stringstream buffer;
    buffer << f.rdbuf();
    contents = std::move(buffer.str());
    current = row = contents.c_str();
    line = 1;
  }

  Scanner(const std::string& src)
  {
    contents = src;
    current = row = contents.c_str();
    line = 1;
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
  char escape_sequence(const char** current);

  char peek(int offset = 0)
  {
    if(current + offset >= contents.c_str() + contents.size()) {
      return '\0';
    }
    return *(current + offset);
  }

  void expect(char ch, const std::string& err_msg)
  {
    if(*current != ch) {
      error(err_msg);
    }
    current++;
  }

  bool match(char ch)
  {
    if(*current == ch) {
      current++;
      return true;
    }
    return false;
  }

  void error(const std::string& err_msg)
  {
    throw std::runtime_error(std::format("[line {}] Err: {}\n", line, err_msg));
  }

  Token eof_token()
  {
    return Token(TokenType::eof, "$", Position(line, int(current - row)));
  }

  bool is_eof(const char* current)
  {
    return current >= (contents.c_str() + contents.size());
  }

  void skip_comments()
  {
    while(!is_eof(current) && *current == '/' && peek(1) == '*') {
      skip_multiline_comment();
    }
  }

  void skip_whitespaces()
  {
    while(!is_eof(current) && std::isspace(*current)) {
      if(*current == '\n') {
        line++;
        row = current + 1;
      }
      current++;
    }
  }

  private:
  std::string contents;
  int line;
  const char* row;
  const char* current;
};

} // namespace lexer