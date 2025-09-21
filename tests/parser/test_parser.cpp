#include <doctest/doctest.h>
#include <filesystem>
#include <lexer/lex.hpp>
#include <parser/parser.hpp>
#include <sstream>
#include <string>
#include <symbol.hpp>

TEST_SUITE("parser")
{

#define SHOULD_PASS(filename)                                                                      \
  TEST_CASE(filename)                                                                              \
  {                                                                                                \
    lexer::Scanner scanner(std::filesystem::path("../tests/parser/valid/" filename));              \
    auto string_table = symbol::StringTable();                                                     \
    std::ostringstream serr;                                                                       \
    parser::Parser parser(serr, scanner, string_table);                                            \
    parser.parse();                                                                                \
    CHECK_FALSE(parser.had_error());                                                               \
  }

#define SHOULD_FAIL(filename, msgs)                                                                \
  TEST_CASE(filename)                                                                              \
  {                                                                                                \
    lexer::Scanner scanner(std::filesystem::path("../tests/parser/invalid/" filename));            \
    auto string_table = symbol::StringTable();                                                     \
    std::ostringstream serr;                                                                       \
    parser::Parser parser(serr, scanner, string_table);                                            \
    parser.parse();                                                                                \
    CHECK(parser.had_error());                                                                     \
    CHECK(serr.str() == msgs);                                                                     \
  }

  SHOULD_PASS("arrays.tig");
  SHOULD_PASS("assign_expr.tig");
  SHOULD_PASS("binary_expr.tig");
  SHOULD_PASS("break_expr.tig");
  SHOULD_PASS("call_expr.tig");
  SHOULD_PASS("for_expr.tig");
  SHOULD_PASS("func_decl.tig");
  SHOULD_PASS("if_expr.tig");
  SHOULD_PASS("record_expr.tig");
  SHOULD_PASS("record_field.tig");
  SHOULD_PASS("sequencing.tig");
  SHOULD_PASS("type_decl.tig");
  SHOULD_PASS("unary_expr.tig");
  SHOULD_PASS("var_decl.tig");
  SHOULD_PASS("while_expr.tig");

  TEST_CASE("valid_book_examples")
  {
    /* These should all pass the syntax check, except test49.tig */

    for(auto& file : std::filesystem::directory_iterator(std::filesystem::path("../tests/book/")))
    {
      auto file_path = file.path();

      if(file_path.filename().string() == "test49.tig")
      {
        continue;
      }

      CHECK_NOTHROW_MESSAGE(
        {
          lexer::Scanner scanner(file.path());
          auto string_table = symbol::StringTable();
          std::ostringstream serr;
          parser::Parser parser(serr, scanner, string_table);
          parser.parse();
        },
        (std::string("file: ") + file_path.generic_string()));
    }
  }

  TEST_CASE("invalid_book_examples")
  {
    auto path = std::filesystem::path("../tests/book/test49.tig");
    lexer::Scanner scanner(path);
    auto string_table = symbol::StringTable();
    std::ostringstream serr;
    parser::Parser parser(serr, scanner, string_table);
    parser.parse();
    CHECK(parser.had_error());
    CHECK(serr.str() == "[../tests/book/test49.tig:5:21] Err at 'nil': expected 'in' after let decls\n");
  }

  SHOULD_FAIL("empty.tig", "[../tests/parser/invalid/empty.tig:1:1] Err at '$': expected expression\n");

  SHOULD_FAIL("junk.tig", "[../tests/parser/invalid/junk.tig:7:1] Err at 'junk': unexpected token after expression\n");

  SHOULD_FAIL("seq.tig", "[../tests/parser/invalid/seq.tig:4:4] Err at '$': expected ')' closing a sequence expression\n");

  SHOULD_FAIL("record.tig",
              "[../tests/parser/invalid/record.tig:4:3] Err at 'var': expected record field name\n"
              "[../tests/parser/invalid/record.tig:5:3] Err at 'var': expected '=' after record field name\n"
              "[../tests/parser/invalid/record.tig:6:3] Err at 'var': expected '}' after record fields\n"
              "[../tests/parser/invalid/record.tig:7:3] Err at 'var': expected expression\n"
              "[../tests/parser/invalid/record.tig:8:15] Err at '{': expected identifier as record type\n"
              "[../tests/parser/invalid/record.tig:10:4] Err at '.': expected variable before token '.'\n"
              "[../tests/parser/invalid/record.tig:12:1] Err at 'end': expected record field name after token '.'\n");

  SHOULD_FAIL("array.tig",
              "[../tests/parser/invalid/array.tig:4:13] Err at '[': expected variable before token '['\n"
              "[../tests/parser/invalid/array.tig:5:15] Err at '[': expected type identifier before token '[' of "
              "array "
              "expression\n"
              "[../tests/parser/invalid/array.tig:7:6] Err at ';': expected ']' closing subscript expression\n"
              "[../tests/parser/invalid/array.tig:8:4] Err at '[': expected variable before token '['\n");

  SHOULD_FAIL("assign.tig",
              "[../tests/parser/invalid/assign.tig:4:5] Err at ':=': invalid assignment target\n"
              "[../tests/parser/invalid/assign.tig:6:7] Err at ':=': invalid assignment target\n"
              "[../tests/parser/invalid/assign.tig:7:7] Err at ':=': invalid assignment target\n");

  SHOULD_FAIL("assign.tig",
              "[../tests/parser/invalid/assign.tig:4:5] Err at ':=': invalid assignment target\n"
              "[../tests/parser/invalid/assign.tig:6:7] Err at ':=': invalid assignment target\n"
              "[../tests/parser/invalid/assign.tig:7:7] Err at ':=': invalid assignment target\n");

  SHOULD_FAIL("binary.tig",
              "[../tests/parser/invalid/binary.tig:2:9] Err at '>': cannot chain comparison operators\n"
              "[../tests/parser/invalid/binary.tig:3:9] Err at '=': cannot chain comparison operators\n"
              "[../tests/parser/invalid/binary.tig:4:10] Err at '<=': cannot chain comparison operators\n");

  SHOULD_FAIL("call.tig",
              "[../tests/parser/invalid/call.tig:4:4] Err at '(': expected identifier as function name\n"
              "[../tests/parser/invalid/call.tig:5:6] Err at '(': expected identifier as function name\n"
              "[../tests/parser/invalid/call.tig:8:1] Err at 'end': expected ')' after function arguments\n");

  SHOULD_FAIL("for.tig",
              "[../tests/parser/invalid/for.tig:2:6] Err at ';': expected identifier\n"
              "[../tests/parser/invalid/for.tig:3:8] Err at ';': expected ':='\n"
              "[../tests/parser/invalid/for.tig:4:11] Err at ';': expected expression\n"
              "[../tests/parser/invalid/for.tig:6:13] Err at ';': expected 'to'\n"
              "[../tests/parser/invalid/for.tig:7:18] Err at ';': expected 'do'\n");

  SHOULD_FAIL("let.tig",
              "[../tests/parser/invalid/let.tig:3:13] Err at ';': expected 'in' after let decls\n"
              "[../tests/parser/invalid/let.tig:7:1] Err at ')': expected expression\n"
              "[../tests/parser/invalid/let.tig:7:2] Err at '$': expected 'end' after let expression\n"
              "[../tests/parser/invalid/let.tig:7:2] Err at '$': expected ')' closing a sequence expression\n");

  SHOULD_FAIL("if.tig",
              "[../tests/parser/invalid/if.tig:2:7] Err at ';': expected 'then' after if condition\n"
              "[../tests/parser/invalid/if.tig:3:12] Err at ';': expected expression\n"
              "[../tests/parser/invalid/if.tig:6:1] Err at ')': expected expression\n"
              "[../tests/parser/invalid/if.tig:6:2] Err at '$': expected ')' closing a sequence expression\n");

  SHOULD_FAIL("while.tig", "[../tests/parser/invalid/while.tig:1:8] Err at '$': expected 'do' after while condition\n");

  SHOULD_FAIL("var.tig",
              "[../tests/parser/invalid/var.tig:3:3] Err at 'var': expected variable identifier\n"
              "[../tests/parser/invalid/var.tig:3:7] Err at ':': expected variable identifier\n"
              "[../tests/parser/invalid/var.tig:4:9] Err at '=': expected ':=' in a variable declaration\n");

  SHOULD_FAIL("func.tig",
              "[../tests/parser/invalid/func.tig:3:3] Err at 'function': expected function name\n"
              "[../tests/parser/invalid/func.tig:4:3] Err at 'function': expected '(' in function declaration\n"
              "[../tests/parser/invalid/func.tig:5:3] Err at 'function': expected parameter name\n"
              "[../tests/parser/invalid/func.tig:6:3] Err at 'function': expected ':' after parameter name\n"
              "[../tests/parser/invalid/func.tig:7:3] Err at 'function': expected parameter type\n"
              "[../tests/parser/invalid/func.tig:8:3] Err at 'function': expected ')' in function declaration\n"
              "[../tests/parser/invalid/func.tig:9:3] Err at 'function': expected function return type\n"
              "[../tests/parser/invalid/func.tig:10:3] Err at 'function': expected '=' before function body\n"
              "[../tests/parser/invalid/func.tig:12:1] Err at 'in': expected expression\n"
              "[../tests/parser/invalid/func.tig:13:4] Err at '$': expected 'in' after let decls\n");

  SHOULD_FAIL("type.tig",
              "[../tests/parser/invalid/type.tig:4:3] Err at 'type': expected type name after token 'type'\n"
              "[../tests/parser/invalid/type.tig:5:3] Err at 'type': expected '=' after type identifier\n"
              "[../tests/parser/invalid/type.tig:6:3] Err at 'type': expected type identifier after '=' token\n"
              "[../tests/parser/invalid/type.tig:8:3] Err at 'type': expected field name\n"
              "[../tests/parser/invalid/type.tig:8:14] Err at '}': expected ':' after field name\n"
              "[../tests/parser/invalid/type.tig:9:16] Err at '}': expected field type after token ':'\n"
              "[../tests/parser/invalid/type.tig:11:1] Err at 'in': expected '}' after type fields\n"
              "[../tests/parser/invalid/type.tig:16:3] Err at 'type': expected 'of' after 'array' token\n"
              "[../tests/parser/invalid/type.tig:17:3] Err at 'type': expected type identifier after 'of' token\n");
}