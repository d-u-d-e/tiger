#include <doctest/doctest.h>
#include <lexer/lex.hpp>
#include <parser/parser.hpp>

TEST_SUITE_BEGIN("parser");

TEST_CASE("sequencing.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/sequencing.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("record_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/record_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("record_field.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/record_field.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_SUITE_END();