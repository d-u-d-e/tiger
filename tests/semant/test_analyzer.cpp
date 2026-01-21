#include "lexer/lex.hpp"
#include "mock_frame.hpp"
#include "parser/parser.hpp"
#include "semant/analyzer.hpp"
#include "string_table.hpp"
#include <array>
#include <doctest/doctest.h>
#include <filesystem>
#include <sstream>
#include <string>

TEST_SUITE("semant_analyzer")
{

#define SHOULD_PASS(filename)                                                                      \
  TEST_CASE(filename)                                                                              \
  {                                                                                                \
    lexer::Scanner scanner(std::filesystem::path(filename));                                       \
    auto string_table = StringTable();                                                             \
    std::ostringstream serr;                                                                       \
    parser::Parser parser(serr, scanner, string_table);                                            \
    auto exp = parser.parse();                                                                     \
    CHECK_FALSE(parser.had_error());                                                               \
    ir::Translator<mock::Frame> translator;                                                        \
                                                                                                   \
    CHECK_NOTHROW({                                                                                \
      semant::Analyzer analyzer(filename, string_table, translator);                               \
      analyzer.type_check(*exp);                                                                   \
    });                                                                                            \
  }

  SHOULD_PASS("../tests/semant/valid/array_expr.tig");
  SHOULD_PASS("../tests/semant/valid/assign_expr.tig");
  SHOULD_PASS("../tests/semant/valid/if_expr.tig");
  SHOULD_PASS("../tests/semant/valid/op_expr.tig");
  SHOULD_PASS("../tests/semant/valid/record_expr.tig");
  SHOULD_PASS("../tests/semant/valid/seq_expr.tig");
  SHOULD_PASS("../tests/semant/valid/while_expr.tig");
  SHOULD_PASS("../tests/semant/valid/for_expr.tig");
  SHOULD_PASS("../tests/semant/valid/vars.tig");
  SHOULD_PASS("../tests/semant/valid/funcs.tig");
  SHOULD_PASS("../tests/semant/valid/types.tig");

  TEST_CASE("valid_book_examples")
  {
    std::array filenames = {
      "merge.tig",  "queens.tig", "test1.tig",  "test2.tig",  "test3.tig",
      "test4.tig",  "test5.tig",  "test6.tig",  "test7.tig",  "test8.tig",
      "test12.tig", "test27.tig", "test30.tig", "test37.tig", "test41.tig",
      "test42.tig", "test44.tig", "test46.tig", "test47.tig", "test48.tig",
    };

    for(auto fname : filenames)
    {
      auto f = std::filesystem::path("../tests/book/" + std::string(fname));
      lexer::Scanner scanner(f);
      auto string_table = StringTable();
      std::ostringstream serr;
      parser::Parser parser(serr, scanner, string_table);
      auto exp = parser.parse();
      CHECK_FALSE_MESSAGE(parser.had_error(), fname);
      ir::Translator<mock::Frame> translator;

      CHECK_NOTHROW_MESSAGE(
        {
          semant::Analyzer analyzer(f, string_table, translator);
          analyzer.type_check(*exp);
        },
        fname);
    }
  }

  TEST_CASE("invalid_book_examples")
  {
    std::array filenames = {"test9.tig",  "test10.tig", "test11.tig", "test13.tig", "test14.tig",
                            "test15.tig", "test16.tig", "test17.tig", "test18.tig", "test19.tig",
                            "test20.tig", "test21.tig", "test22.tig", "test23.tig", "test24.tig",
                            "test25.tig", "test26.tig", "test28.tig", "test29.tig", "test31.tig",
                            "test32.tig", "test33.tig", "test34.tig", "test35.tig", "test36.tig",
                            "test38.tig", "test39.tig", "test40.tig", "test43.tig", "test45.tig"};

    // test49.tig has a syntax error

    for(auto fname : filenames)
    {
      auto f = std::filesystem::path("../tests/book/" + std::string(fname));

      lexer::Scanner scanner(f);
      auto string_table = StringTable();
      std::ostringstream serr;
      parser::Parser parser(serr, scanner, string_table);
      auto exp = parser.parse();
      CHECK_FALSE_MESSAGE(parser.had_error(), fname);
      ir::Translator<mock::Frame> translator;
      semant::Analyzer analyzer(f, string_table, translator);
      CHECK_THROWS_MESSAGE(analyzer.type_check(*exp), fname);
    }
  }

#define SHOULD_THROW(filename, msg)                                                                \
  TEST_CASE(filename)                                                                              \
  {                                                                                                \
    auto input = std::filesystem::path(filename);                                                  \
    lexer::Scanner scanner(input);                                                                 \
    auto string_table = StringTable();                                                             \
    std::ostringstream serr;                                                                       \
    parser::Parser parser(serr, scanner, string_table);                                            \
    auto exp = parser.parse();                                                                     \
    CHECK_FALSE(parser.had_error());                                                               \
    ir::Translator<mock::Frame> translator;                                                        \
    CHECK_THROWS_WITH(                                                                             \
      {                                                                                            \
        semant::Analyzer analyzer(input, string_table, translator);                                \
        analyzer.type_check(*exp);                                                                 \
      },                                                                                           \
      msg);                                                                                        \
  }

  // clang-format off
SHOULD_THROW("../tests/semant/invalid/array/elem_access.tig", 
  "[../tests/semant/invalid/array/elem_access.tig:7:4] Err: expression between '[]' must be an integer");

SHOULD_THROW("../tests/semant/invalid/array/init_type_mismatch.tig", 
  "[../tests/semant/invalid/array/init_type_mismatch.tig:7:12] Err: array type mismatch: 'string' != 'int'");

SHOULD_THROW("../tests/semant/invalid/array/size_not_int.tig", 
  "[../tests/semant/invalid/array/size_not_int.tig:7:12] Err: array size must be an integer");

SHOULD_THROW("../tests/semant/invalid/array/undefined.tig", 
  "[../tests/semant/invalid/array/undefined.tig:6:12] Err: undefined array type 'StrArray'");

SHOULD_THROW("../tests/semant/invalid/function/arg_type_mismatch.tig", 
  "[../tests/semant/invalid/function/arg_type_mismatch.tig:7:4] Err: argument 0 expects type 'int', got 'string'");

SHOULD_THROW("../tests/semant/invalid/function/args.tig", 
  "[../tests/semant/invalid/function/args.tig:6:4] Err: expected 0 arguments, got 1");

SHOULD_THROW("../tests/semant/invalid/function/redecl.tig", 
  "[../tests/semant/invalid/function/redecl.tig:8:3] Err: redeclaration of function 'g'");

SHOULD_THROW("../tests/semant/invalid/function/return_body_mismatch.tig", 
  "[../tests/semant/invalid/function/return_body_mismatch.tig:5:24] Err: return type 'int' does not match body type 'string'");

SHOULD_THROW("../tests/semant/invalid/function/undef_param_type.tig", 
  "[../tests/semant/invalid/function/undef_param_type.tig:6:14] Err: undefined parameter type 'U'");

SHOULD_THROW("../tests/semant/invalid/function/undef_return_type.tig", 
  "[../tests/semant/invalid/function/undef_return_type.tig:5:18] Err: undefined return type 'T'");

SHOULD_THROW("../tests/semant/invalid/function/undefined.tig", 
  "[../tests/semant/invalid/function/undefined.tig:6:11] Err: undefined function 'g'");

SHOULD_THROW("../tests/semant/invalid/if/cond_not_int.tig", 
  "[../tests/semant/invalid/if/cond_not_int.tig:3:1] Err: the condition must be an integer");

SHOULD_THROW("../tests/semant/invalid/if/then_else_mismatch.tig", 
  "[../tests/semant/invalid/if/then_else_mismatch.tig:6:3] Err: types of then and else branches must match");

SHOULD_THROW("../tests/semant/invalid/if/then_value.tig", 
  "[../tests/semant/invalid/if/then_value.tig:3:1] Err: the then branch must not produce any value");

SHOULD_THROW("../tests/semant/invalid/loops/break_outside.tig", 
  "[../tests/semant/invalid/loops/break_outside.tig:20:3] Err: break statement not within a loop");

SHOULD_THROW("../tests/semant/invalid/loops/break_outside2.tig", 
  "[../tests/semant/invalid/loops/break_outside2.tig:7:9] Err: break statement not within a loop");

SHOULD_THROW("../tests/semant/invalid/loops/for_body_value.tig", 
  "[../tests/semant/invalid/loops/for_body_value.tig:3:1] Err: the body of the for loop must not produce any value");

SHOULD_THROW("../tests/semant/invalid/loops/for_high.tig", 
  "[../tests/semant/invalid/loops/for_high.tig:3:1] Err: the upper bound must be an integer");

SHOULD_THROW("../tests/semant/invalid/loops/for_low.tig", 
  "[../tests/semant/invalid/loops/for_low.tig:3:1] Err: the lower bound must be an integer");

SHOULD_THROW("../tests/semant/invalid/loops/while_body_value.tig", 
  "[../tests/semant/invalid/loops/while_body_value.tig:3:1] Err: the body of the while loop must not produce any value");

SHOULD_THROW("../tests/semant/invalid/loops/while_cond.tig", 
  "[../tests/semant/invalid/loops/while_cond.tig:3:1] Err: the condition must be an integer");

SHOULD_THROW("../tests/semant/invalid/op/array_eq_nil.tig", 
  "[../tests/semant/invalid/op/array_eq_nil.tig:7:5] Err: invalid operand types");

SHOULD_THROW("../tests/semant/invalid/op/assign.tig", 
  "[../tests/semant/invalid/op/assign.tig:7:5] Err: cannot assign 'string' to 'int'");

SHOULD_THROW("../tests/semant/invalid/op/dot.tig", 
  "[../tests/semant/invalid/op/dot.tig:10:5] Err: 'string' is not a record type");

SHOULD_THROW("../tests/semant/invalid/op/int_eq_str.tig", 
  "[../tests/semant/invalid/op/int_eq_str.tig:4:5] Err: invalid operand types");

SHOULD_THROW("../tests/semant/invalid/op/int_plus_str.tig", 
  "[../tests/semant/invalid/op/int_plus_str.tig:4:5] Err: invalid operand types");

SHOULD_THROW("../tests/semant/invalid/op/record_eq_array.tig", 
  "[../tests/semant/invalid/op/record_eq_array.tig:9:5] Err: invalid operand types");

SHOULD_THROW("../tests/semant/invalid/op/subscript.tig", 
  "[../tests/semant/invalid/op/subscript.tig:10:4] Err: 'string' is not an array type");

SHOULD_THROW("../tests/semant/invalid/record/dot_unexpected_field_name.tig", 
  "[../tests/semant/invalid/record/dot_unexpected_field_name.tig:7:5] Err: unexpected record field name 'c'");

SHOULD_THROW("../tests/semant/invalid/record/fields.tig", 
  "[../tests/semant/invalid/record/fields.tig:6:3] Err: expected 2 fields, got 1");
  
SHOULD_THROW("../tests/semant/invalid/record/undefined.tig", 
  "[../tests/semant/invalid/record/undefined.tig:6:3] Err: undefined record type 'S'");

SHOULD_THROW("../tests/semant/invalid/record/unexpected_field_name.tig", 
  "[../tests/semant/invalid/record/unexpected_field_name.tig:6:13] Err: expected field 'b', got 'c'");

SHOULD_THROW("../tests/semant/invalid/record/unexpected_field_type.tig", 
  "[../tests/semant/invalid/record/unexpected_field_type.tig:6:13] Err: expected type 'string' for field 'b', got 'int'");

SHOULD_THROW("../tests/semant/invalid/type/cycle1.tig", 
  "[../tests/semant/invalid/type/cycle1.tig:4:3] Err: cycle in type declaration");

SHOULD_THROW("../tests/semant/invalid/type/cycle2.tig", 
  "[../tests/semant/invalid/type/cycle2.tig:4:3] Err: cycle in type declaration");

SHOULD_THROW("../tests/semant/invalid/type/cycle3.tig", 
  "[../tests/semant/invalid/type/cycle3.tig:4:3] Err: cycle in type declaration");

SHOULD_THROW("../tests/semant/invalid/type/redecl.tig", 
  "[../tests/semant/invalid/type/redecl.tig:8:3] Err: redeclaration of type 'A'");
  
SHOULD_THROW("../tests/semant/invalid/type/undef_array.tig", 
  "[../tests/semant/invalid/type/undef_array.tig:6:21] Err: undefined type 'T'");

SHOULD_THROW("../tests/semant/invalid/type/undef_name.tig", 
  "[../tests/semant/invalid/type/undef_name.tig:5:12] Err: undefined type 'T'");

SHOULD_THROW("../tests/semant/invalid/type/undef_record.tig", 
  "[../tests/semant/invalid/type/undef_record.tig:4:21] Err: undefined type 'T'");

SHOULD_THROW("../tests/semant/invalid/var/init_nil.tig", 
  "[../tests/semant/invalid/var/init_nil.tig:6:3] Err: nil must be constrained by a record type");

SHOULD_THROW("../tests/semant/invalid/var/init_type_mismatch.tig", 
  "[../tests/semant/invalid/var/init_type_mismatch.tig:5:11] Err: decl type 'R' does not match expr type 'int'");

SHOULD_THROW("../tests/semant/invalid/var/undef_type.tig", 
  "[../tests/semant/invalid/var/undef_type.tig:5:11] Err: undefined type 'R'");

SHOULD_THROW("../tests/semant/invalid/var/undefined.tig", 
  "[../tests/semant/invalid/var/undefined.tig:3:5] Err: undefined variable 'a'");
  // clang-format on
}