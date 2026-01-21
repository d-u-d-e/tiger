#pragma once
#include "lexer/token.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>

namespace lexer
{

class Exception : public std::runtime_error
{
  public:
  Exception(const std::string& what)
    : std::runtime_error(what)
  { }
};

class Scanner
{
  public:
  Scanner(const std::filesystem::path& filename);
  Scanner(const std::string& src);

  std::string filename();
  Token next();

  private:
  Token read_token();
  Token identifier();
  Token string_literal();
  Token integer_literal();
  Token punctuation();
  void skip_multiline_comment();
  std::string escape_sequence();
  char peek(int offset = 0);
  void expect(char ch, const std::string& err_msg);
  bool match(char ch);
  void error_at(const std::string& err_msg);
  void error(const std::string& err_msg);
  Token eof_token();
  bool is_eof(const char* current);
  void skip_comments();
  void skip_whitespaces();

  private:
  std::string filename_;
  std::string contents;
  int line;
  const char* row;
  const char* current;
};

} // namespace lexer