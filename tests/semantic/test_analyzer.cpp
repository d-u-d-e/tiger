#include <doctest/doctest.h>
#include <lexer/lex.hpp>
#include <parser/parser.hpp>
#include <semantic/analyzer.hpp>

TEST_SUITE_BEGIN("semantic_analyzer");

#define SHOULD_PASS(filename)                                                  \
  TEST_CASE(filename)                                                          \
  {                                                                            \
    lexer::Scanner scanner(                                                    \
      std::filesystem::path("../tests/semantic/valid/" filename));             \
    auto string_table = symbol::StringTable();                                 \
    parser::Parser parser(scanner, string_table);                              \
                                                                               \
    CHECK_NOTHROW({                                                            \
      auto exp = parser.parse();                                               \
      semantic::Analyzer analyzer(string_table);                               \
      analyzer.type_check(*exp);                                               \
    });                                                                        \
  }

SHOULD_PASS("array_expr.tig");
SHOULD_PASS("assign_expr.tig");
SHOULD_PASS("if_expr.tig");
SHOULD_PASS("op_expr.tig");
SHOULD_PASS("record_expr.tig");
SHOULD_PASS("seq_expr.tig");
SHOULD_PASS("while_expr.tig");
SHOULD_PASS("for_expr.tig");
SHOULD_PASS("vars.tig");
SHOULD_PASS("funcs.tig");
SHOULD_PASS("types.tig");

TEST_SUITE_END();