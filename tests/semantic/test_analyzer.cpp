#include <doctest/doctest.h>
#include <lexer/lex.hpp>
#include <parser/parser.hpp>
#include <semantic/analyzer.hpp>

TEST_SUITE_BEGIN("semantic_analyzer");

TEST_CASE("assign_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/semantic/valid/assign_expr.tig"));
  auto string_table = symbol::StringTable();
  parser::Parser parser(scanner, string_table);

  CHECK_NOTHROW({
    auto exp = parser.parse();
    semantic::Analyzer analyzer(string_table);
    analyzer.type_check(*exp);
  });
}

TEST_CASE("op_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/semantic/valid/seq_expr.tig"));
  auto string_table = symbol::StringTable();
  parser::Parser parser(scanner, string_table);

  CHECK_NOTHROW({
    auto exp = parser.parse();
    semantic::Analyzer analyzer(string_table);
    analyzer.type_check(*exp);
  });
}

TEST_CASE("array_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/semantic/valid/array_expr.tig"));
  auto string_table = symbol::StringTable();
  parser::Parser parser(scanner, string_table);

  CHECK_NOTHROW({
    auto exp = parser.parse();
    semantic::Analyzer analyzer(string_table);
    analyzer.type_check(*exp);
  });
}

TEST_CASE("record_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/semantic/valid/record_expr.tig"));
  auto string_table = symbol::StringTable();
  parser::Parser parser(scanner, string_table);

  CHECK_NOTHROW({
    auto exp = parser.parse();
    semantic::Analyzer analyzer(string_table);
    analyzer.type_check(*exp);
  });
}

TEST_CASE("if_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/semantic/valid/if_expr.tig"));
  auto string_table = symbol::StringTable();
  parser::Parser parser(scanner, string_table);

  CHECK_NOTHROW({
    auto exp = parser.parse();
    semantic::Analyzer analyzer(string_table);
    analyzer.type_check(*exp);
  });
}

TEST_CASE("while_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/semantic/valid/while_expr.tig"));
  auto string_table = symbol::StringTable();
  parser::Parser parser(scanner, string_table);

  CHECK_NOTHROW({
    auto exp = parser.parse();
    semantic::Analyzer analyzer(string_table);
    analyzer.type_check(*exp);
  });
}

TEST_SUITE_END();