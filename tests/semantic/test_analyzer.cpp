#include <doctest/doctest.h>
#include <lexer/lex.hpp>
#include <parser/parser.hpp>
#include <semantic/analyzer.hpp>

TEST_SUITE_BEGIN("semantic_analyzer");

TEST_CASE("op_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/semantic/valid/op_expr.tig"));
  auto string_table = symbol::StringTable();
  parser::Parser parser(scanner, string_table);

  CHECK_NOTHROW({
    auto exp = parser.parse();
    semantic::Analyzer analyzer(string_table);
    analyzer.type_check(*exp);
  });
}

TEST_SUITE_END();