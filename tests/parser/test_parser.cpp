#include <doctest/doctest.h>
#include <lexer/lex.hpp>
#include <parser/parser.hpp>
#include <symbol.hpp>

TEST_SUITE_BEGIN("parser");

#define SHOULD_PASS(filename)                                                  \
  TEST_CASE(filename)                                                          \
  {                                                                            \
    lexer::Scanner scanner(                                                    \
      std::filesystem::path("../tests/parser/valid/" filename));               \
    auto string_table = symbol::StringTable();                                 \
    std::ostringstream serr;                                                   \
    parser::Parser parser(serr, scanner, string_table);                        \
    parser.parse();                                                            \
    CHECK_FALSE(parser.had_error());                                           \
  }

#define SHOULD_FAIL(filename, msgs)                                            \
  TEST_CASE(filename)                                                          \
  {                                                                            \
    lexer::Scanner scanner(                                                    \
      std::filesystem::path("../tests/parser/invalid/" filename));             \
    auto string_table = symbol::StringTable();                                 \
    std::ostringstream serr;                                                   \
    parser::Parser parser(serr, scanner, string_table);                        \
    parser.parse();                                                            \
    CHECK(parser.had_error());                                                 \
    CHECK(serr.str() == msgs);                                                 \
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

  for(auto& file : std::filesystem::directory_iterator(
        std::filesystem::path("../tests/book/"))) {
    auto file_path = file.path();

    if(file_path.filename().string() == "test49.tig") {
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
  CHECK(serr.str() ==
        "[line 5:21] Err at 'nil': expected 'in' after let decls\n");
}

SHOULD_FAIL("empty.tig", "[line 1:1] Err at '$': expected expression\n");

SHOULD_FAIL("junk.tig",
            "[line 7:1] Err at 'junk': unexpected token after expression\n");

SHOULD_FAIL(
  "seq.tig",
  "[line 4:4] Err at '$': expected ')' closing a sequence expression\n");

SHOULD_FAIL(
  "record.tig",
  "[line 4:3] Err at 'var': expected record field name\n"
  "[line 5:3] Err at 'var': expected '=' after record field name\n"
  "[line 6:3] Err at 'var': expected '}' after record fields\n"
  "[line 7:3] Err at 'var': expected expression\n"
  "[line 8:15] Err at '{': expected identifier as record type\n"
  "[line 10:4] Err at '.': expected variable before token '.'\n"
  "[line 12:1] Err at 'end': expected record field name after token '.'\n");

SHOULD_FAIL(
  "array.tig",
  "[line 4:13] Err at '[': expected variable before token '['\n"
  "[line 5:15] Err at '[': expected type identifier before token '[' of array "
  "expression\n"
  "[line 7:6] Err at ';': expected ']' closing subscript expression\n"
  "[line 8:4] Err at '[': expected variable before token '['\n");

SHOULD_FAIL("assign.tig",
            "[line 4:5] Err at ':=': invalid assignment target\n"
            "[line 6:7] Err at ':=': invalid assignment target\n"
            "[line 7:7] Err at ':=': invalid assignment target\n");

SHOULD_FAIL("assign.tig",
            "[line 4:5] Err at ':=': invalid assignment target\n"
            "[line 6:7] Err at ':=': invalid assignment target\n"
            "[line 7:7] Err at ':=': invalid assignment target\n");

SHOULD_FAIL("binary.tig",
            "[line 2:9] Err at '>': cannot chain comparison operators\n"
            "[line 3:9] Err at '=': cannot chain comparison operators\n"
            "[line 4:10] Err at '<=': cannot chain comparison operators\n");

SHOULD_FAIL("call.tig",
            "[line 4:4] Err at '(': expected identifier as function name\n"
            "[line 5:6] Err at '(': expected identifier as function name\n"
            "[line 8:1] Err at 'end': expected ')' after function arguments\n");

SHOULD_FAIL("for.tig",
            "[line 2:6] Err at ';': expected identifier\n"
            "[line 3:8] Err at ';': expected ':='\n"
            "[line 4:11] Err at ';': expected expression\n"
            "[line 6:13] Err at ';': expected 'to'\n"
            "[line 7:18] Err at ';': expected 'do'\n");

SHOULD_FAIL(
  "let.tig",
  "[line 3:13] Err at ';': expected 'in' after let decls\n"
  "[line 7:1] Err at ')': expected expression\n"
  "[line 7:2] Err at '$': expected 'end' after let expression\n"
  "[line 7:2] Err at '$': expected ')' closing a sequence expression\n");

TEST_SUITE_END();