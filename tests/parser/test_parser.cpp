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
    parser::Parser parser(scanner, string_table);                              \
    parser.parse();                                                            \
    CHECK_FALSE(parser.had_error());                                           \
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
        parser::Parser parser(scanner, string_table);
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
  parser::Parser parser(scanner, string_table);
  parser.parse();
  CHECK(parser.had_error());
}

TEST_SUITE_END();