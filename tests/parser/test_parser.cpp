#include <doctest/doctest.h>
#include <lexer/lex.hpp>
#include <parser/parser.hpp>
#include <symbol.hpp>

TEST_SUITE_BEGIN("parser");

TEST_CASE("sequencing.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/sequencing.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("record_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/record_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("record_field.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/record_field.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("arrays.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/arrays.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("while_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/while_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("for_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/for_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("break_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/break_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("func_decl.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/func_decl.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("type_decl.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/type_decl.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("var_decl.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/var_decl.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("if_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/if_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("binary_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/binary_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("unary_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/unary_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("assign_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/assign_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("call_expr.tig")
{
  lexer::Scanner scanner(
    std::filesystem::path("../tests/parser/valid/call_expr.tig"));
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_NOTHROW(parser.parse());
}

TEST_CASE("valid_book_examples")
{
  /* These should all pass the syntax check, except test49.tig */
  CHECK_NOTHROW(for(auto& file
                    : std::filesystem::directory_iterator(
                      std::filesystem::path("../tests/book/"))) {
    auto file_path = file.path();

    if(file_path.filename().string() == "test49.tig") {
      continue;
    }
    lexer::Scanner scanner(file.path());
    auto symbol_table = symbol::SymbolTable();
    parser::Parser parser(scanner, symbol_table);
    parser.parse();
  });
}

TEST_CASE("invalid_book_examples")
{
  auto path = std::filesystem::path("../tests/book/test49.tig");
  lexer::Scanner scanner(path);
  auto symbol_table = symbol::SymbolTable();
  parser::Parser parser(scanner, symbol_table);
  CHECK_THROWS(parser.parse());
}

TEST_SUITE_END();