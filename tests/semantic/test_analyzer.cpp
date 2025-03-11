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

#define SHOULD_THROW(filename, msg)                                            \
  TEST_CASE(filename)                                                          \
  {                                                                            \
    lexer::Scanner scanner(                                                    \
      std::filesystem::path("../tests/semantic/invalid/" filename));           \
    auto string_table = symbol::StringTable();                                 \
    parser::Parser parser(scanner, string_table);                              \
                                                                               \
    CHECK_THROWS_WITH(                                                         \
      {                                                                        \
        auto exp = parser.parse();                                             \
        semantic::Analyzer analyzer(string_table);                             \
        analyzer.type_check(*exp);                                             \
      },                                                                       \
      msg);                                                                    \
  }

// clang-format off

SHOULD_THROW("array/elem_access.tig", "[line 7:4] Err: 'int' is not an array type");
SHOULD_THROW("array/init_type_mismatch.tig", "[line 7:12] Err: array type mismatch: 'string' != 'int'");
SHOULD_THROW("array/size_not_int.tig", "[line 7:12] Err: array size must be an integer");
SHOULD_THROW("array/undefined.tig", "[line 6:12] Err: undefined array type 'StrArray'");

SHOULD_THROW("function/arg_type_mismatch.tig", "[line 7:4] Err: argument 0 expects type 'int', got 'string'");
SHOULD_THROW("function/args.tig", "[line 6:4] Err: expected 0 arguments, got 1");
SHOULD_THROW("function/redecl.tig", "[line 8:3] Err: redeclaration of function 'g'");
SHOULD_THROW("function/return_body_mismatch.tig", "[line 5:24] Err: return type 'int' does not match body type 'string'");
SHOULD_THROW("function/undef_param_type.tig", "[line 6:14] Err: undefined parameter type 'U'");
SHOULD_THROW("function/undef_return_type.tig", "[line 5:18] Err: undefined return type 'T'");
SHOULD_THROW("function/undefined.tig", "[line 6:11] Err: undefined function 'g'");

SHOULD_THROW("if/cond_not_int.tig", "[line 3:1] Err: the condition must be an integer");
SHOULD_THROW("if/then_else_mismatch.tig", "[line 6:3] Err: types of then and else branches must match");
SHOULD_THROW("if/then_value.tig", "[line 3:1] Err: the then branch must not produce any value");

SHOULD_THROW("loops/break_outside.tig", "[line 20:3] Err: break statement not within a loop");
SHOULD_THROW("loops/for_body_value.tig", "[line 3:1] Err: the body of the for loop must not produce any value");
SHOULD_THROW("loops/for_high.tig", "[line 3:1] Err: the upper bound must be an integer");
SHOULD_THROW("loops/for_low.tig", "[line 3:1] Err: the lower bound must be an integer");
SHOULD_THROW("loops/while_body_value.tig", "[line 3:1] Err: the body of the while loop must not produce any value");
SHOULD_THROW("loops/while_cond.tig", "[line 3:1] Err: the condition must be an integer");

SHOULD_THROW("op/array_eq_nil.tig", "[line 7:5] Err: operands must be of the same type");
SHOULD_THROW("op/dot.tig", "[line 10:5] Err: 'string' is not a record type");
SHOULD_THROW("op/int_eq_str.tig", "[line 4:5] Err: operands must be of the same type");
SHOULD_THROW("op/int_plus_str.tig", "[line 4:5] Err: operands must be integers");
SHOULD_THROW("op/record_eq_array.tig", "[line 9:5] Err: operands must be of the same type");
SHOULD_THROW("op/subscript.tig", "[line 10:4] Err: 'string' is not an array type");



// clang-format on
TEST_SUITE_END();