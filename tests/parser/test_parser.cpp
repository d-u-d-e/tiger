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

TEST_CASE("arrays.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/arrays.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("while_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/while_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("for_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/for_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("break_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/break_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("func_decl.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/func_decl.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("type_decl.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/type_decl.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("var_decl.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/var_decl.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("if_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/if_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("binary_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/binary_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("unary_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/unary_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("assign_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/assign_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("call_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/call_expr.tig"));
  parser::Parser parser(scanner);
  CHECK_NOTHROW(parser.parse());
}

TEST_SUITE_END();