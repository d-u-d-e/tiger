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
  explicit Exception(const std::string& what)
    : std::runtime_error(what)
  { }
};

class Scanner
{
  public:
  explicit Scanner(const std::filesystem::path& filename);
  explicit Scanner(std::string src);

  auto filename() -> std::string;
  auto next() -> Token;

  private:
  auto read_token() -> Token;
  auto identifier() -> Token;
  auto string_literal() -> Token;
  auto integer_literal() -> Token;
  auto punctuation() -> Token;
  void skip_multiline_comment();
  auto escape_sequence() -> std::string;
  auto peek(int offset = 0) -> char;
  void expect(char ch, const std::string& err_msg);
  auto match(char ch) -> bool;
  void error_at(const std::string& err_msg);
  void static error(const std::string& err_msg);
  auto eof_token() -> Token;
  auto is_eof(const char* current) -> bool;
  void skip_comments();
  void skip_whitespaces();

  std::string filename_;
  std::string contents;
  int line;
  const char* row{};
  const char* current{};
};

} // namespace lexer