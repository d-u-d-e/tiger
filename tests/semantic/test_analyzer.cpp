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

TEST_CASE("valid_book_examples")
{
  std::array<std::string, 20> filenames = {
    "merge.tig",  "queens.tig", "test1.tig",  "test2.tig",  "test3.tig",
    "test4.tig",  "test5.tig",  "test6.tig",  "test7.tig",  "test8.tig",
    "test12.tig", "test27.tig", "test30.tig", "test37.tig", "test41.tig",
    "test42.tig", "test44.tig", "test46.tig", "test47.tig", "test48.tig",
  };

  for(auto& fname : filenames) {
    auto f = std::filesystem::path("../tests/book/" + fname);
    CHECK_NOTHROW_MESSAGE(
      {
        lexer::Scanner scanner(f);
        auto string_table = symbol::StringTable();
        parser::Parser parser(scanner, string_table);
        auto exp = parser.parse();
        semantic::Analyzer analyzer(string_table);
        analyzer.type_check(*exp);
      },
      (std::string("file ") + f.generic_string()));
  }
}

TEST_CASE("invalid_book_examples")
{
  std::array<std::string, 31> filenames = {
    "test9.tig",  "test10.tig", "test11.tig", "test13.tig", "test14.tig",
    "test15.tig", "test16.tig", "test17.tig", "test18.tig", "test19.tig",
    "test20.tig", "test21.tig", "test22.tig", "test23.tig", "test24.tig",
    "test25.tig", "test26.tig", "test28.tig", "test29.tig", "test31.tig",
    "test32.tig", "test33.tig", "test34.tig", "test35.tig", "test36.tig",
    "test38.tig", "test39.tig", "test40.tig", "test43.tig", "test45.tig",
    "test49.tig"};

  for(auto& fname : filenames) {
    auto f = std::filesystem::path("../tests/book/" + fname);
    CHECK_THROWS_MESSAGE(
      {
        lexer::Scanner scanner(f);
        auto string_table = symbol::StringTable();
        parser::Parser parser(scanner, string_table);
        auto exp = parser.parse();
        semantic::Analyzer analyzer(string_table);
        analyzer.type_check(*exp);
      },
      (std::string("file ") + f.generic_string()));
  }
}

TEST_SUITE_END();