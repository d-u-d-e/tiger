#include <doctest/doctest.h>
#include <lexer/lex.hpp>

TEST_SUITE_BEGIN("lexer");

TEST_CASE("queens.tig")
{
  lexer::Scanner scanner(std::filesystem::path("../tests/book/queens.tig"));

  const lexer::Token tokens[] = {
    // clang-format off
    {lexer::TokenType::let_keyword, "let", lexer::Position(3, 1)},

    {lexer::TokenType::var_keyword, "var", lexer::Position(4, 5)},
    {lexer::TokenType::identifier, "N", lexer::Position(4, 9)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(4, 11)},
    {lexer::TokenType::integer_literal, "8", lexer::Position(4, 14)},

    {lexer::TokenType::type_keyword, "type", lexer::Position(6, 5)},
    {lexer::TokenType::identifier, "intArray", lexer::Position(6, 10)},
    {lexer::TokenType::equal_op, "=", lexer::Position(6, 19)},
    {lexer::TokenType::array_keyword, "array", lexer::Position(6, 21)},
    {lexer::TokenType::of_keyword, "of", lexer::Position(6, 27)},
    {lexer::TokenType::identifier, "int", lexer::Position(6, 30)},

    {lexer::TokenType::var_keyword, "var", lexer::Position(8, 5)},
    {lexer::TokenType::identifier, "row", lexer::Position(8, 9)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(8, 13)},
    {lexer::TokenType::identifier, "intArray", lexer::Position(8, 16)},
    {lexer::TokenType::lbracket, "[", lexer::Position(8, 25)},
    {lexer::TokenType::identifier, "N", lexer::Position(8, 27)},
    {lexer::TokenType::rbracket, "]", lexer::Position(8, 29)},
    {lexer::TokenType::of_keyword, "of", lexer::Position(8, 31)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(8, 34)},

    {lexer::TokenType::var_keyword, "var", lexer::Position(9, 5)},
    {lexer::TokenType::identifier, "col", lexer::Position(9, 9)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(9, 13)},
    {lexer::TokenType::identifier, "intArray", lexer::Position(9, 16)},
    {lexer::TokenType::lbracket, "[", lexer::Position(9, 25)},
    {lexer::TokenType::identifier, "N", lexer::Position(9, 27)},
    {lexer::TokenType::rbracket, "]", lexer::Position(9, 29)},
    {lexer::TokenType::of_keyword, "of", lexer::Position(9, 31)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(9, 34)},

    {lexer::TokenType::var_keyword, "var", lexer::Position(10, 5)},
    {lexer::TokenType::identifier, "diag1", lexer::Position(10, 9)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(10, 15)},
    {lexer::TokenType::identifier, "intArray", lexer::Position(10, 18)},
    {lexer::TokenType::lbracket, "[", lexer::Position(10, 27)},
    {lexer::TokenType::identifier, "N", lexer::Position(10, 28)},
    {lexer::TokenType::plus_op, "+", lexer::Position(10, 29)},
    {lexer::TokenType::identifier, "N", lexer::Position(10, 30)},
    {lexer::TokenType::minus_op, "-", lexer::Position(10, 31)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(10, 32)},
    {lexer::TokenType::rbracket, "]", lexer::Position(10, 33)},
    {lexer::TokenType::of_keyword, "of", lexer::Position(10, 35)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(10, 38)},

    {lexer::TokenType::var_keyword, "var", lexer::Position(11, 5)},
    {lexer::TokenType::identifier, "diag2", lexer::Position(11, 9)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(11, 15)},
    {lexer::TokenType::identifier, "intArray", lexer::Position(11, 18)},
    {lexer::TokenType::lbracket, "[", lexer::Position(11, 27)},
    {lexer::TokenType::identifier, "N", lexer::Position(11, 28)},
    {lexer::TokenType::plus_op, "+", lexer::Position(11, 29)},
    {lexer::TokenType::identifier, "N", lexer::Position(11, 30)},
    {lexer::TokenType::minus_op, "-", lexer::Position(11, 31)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(11, 32)},
    {lexer::TokenType::rbracket, "]", lexer::Position(11, 33)},
    {lexer::TokenType::of_keyword, "of", lexer::Position(11, 35)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(11, 38)},

    {lexer::TokenType::function_keyword, "function", lexer::Position(13, 5)},
    {lexer::TokenType::identifier, "printboard", lexer::Position(13, 14)},
    {lexer::TokenType::lparen, "(", lexer::Position(13, 24)},
    {lexer::TokenType::rparen, ")", lexer::Position(13, 25)},
    {lexer::TokenType::equal_op, "=", lexer::Position(13, 27)},

    {lexer::TokenType::lparen, "(", lexer::Position(14, 8)},
    {lexer::TokenType::for_keyword, "for", lexer::Position(14, 9)},
    {lexer::TokenType::identifier, "i", lexer::Position(14, 13)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(14, 15)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(14, 18)},
    {lexer::TokenType::to_keyword, "to", lexer::Position(14, 20)},
    {lexer::TokenType::identifier, "N", lexer::Position(14, 23)},
    {lexer::TokenType::minus_op, "-", lexer::Position(14, 24)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(14, 25)},

    {lexer::TokenType::do_keyword, "do", lexer::Position(15, 7)},
    {lexer::TokenType::lparen, "(", lexer::Position(15, 10)},
    {lexer::TokenType::for_keyword, "for", lexer::Position(15, 11)},
    {lexer::TokenType::identifier, "j", lexer::Position(15, 15)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(15, 17)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(15, 20)},
    {lexer::TokenType::to_keyword, "to", lexer::Position(15, 22)},
    {lexer::TokenType::identifier, "N", lexer::Position(15, 25)},
    {lexer::TokenType::minus_op, "-", lexer::Position(15, 26)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(15, 27)},

    {lexer::TokenType::do_keyword, "do", lexer::Position(16, 12)},
    {lexer::TokenType::identifier, "print", lexer::Position(16, 15)},
    {lexer::TokenType::lparen, "(", lexer::Position(16, 20)},
    {lexer::TokenType::if_keyword, "if", lexer::Position(16, 21)},
    {lexer::TokenType::identifier, "col", lexer::Position(16, 24)},
    {lexer::TokenType::lbracket, "[", lexer::Position(16, 27)},
    {lexer::TokenType::identifier, "i", lexer::Position(16, 28)},
    {lexer::TokenType::rbracket, "]", lexer::Position(16, 29)},
    {lexer::TokenType::equal_op, "=", lexer::Position(16, 30)},
    {lexer::TokenType::identifier, "j", lexer::Position(16, 31)},
    {lexer::TokenType::then_keyword, "then", lexer::Position(16, 33)},
    {lexer::TokenType::string_literal, " O", lexer::Position(16, 38)},
    {lexer::TokenType::else_keyword, "else", lexer::Position(16, 43)},
    {lexer::TokenType::string_literal, " .", lexer::Position(16, 48)},
    {lexer::TokenType::rparen, ")", lexer::Position(16, 52)},
    {lexer::TokenType::semicolon, ";", lexer::Position(16, 53)},

    {lexer::TokenType::identifier, "print", lexer::Position(17, 11)},
    {lexer::TokenType::lparen, "(", lexer::Position(17, 16)},
    {lexer::TokenType::string_literal, "\n", lexer::Position(17, 17)},
    {lexer::TokenType::rparen, ")", lexer::Position(17, 21)},
    {lexer::TokenType::rparen, ")", lexer::Position(17, 22)},
    {lexer::TokenType::semicolon, ";", lexer::Position(17, 23)},

    {lexer::TokenType::identifier, "print", lexer::Position(18, 10)},
    {lexer::TokenType::lparen, "(", lexer::Position(18, 15)},
    {lexer::TokenType::string_literal, "\n", lexer::Position(18, 16)},
    {lexer::TokenType::rparen, ")", lexer::Position(18, 20)},
    {lexer::TokenType::rparen, ")", lexer::Position(18, 21)},

    {lexer::TokenType::function_keyword, "function", lexer::Position(20, 5)},
    {lexer::TokenType::identifier, "try", lexer::Position(20, 14)},
    {lexer::TokenType::lparen, "(", lexer::Position(20, 17)},
    {lexer::TokenType::identifier, "c", lexer::Position(20, 18)},
    {lexer::TokenType::colon, ":", lexer::Position(20, 19)},
    {lexer::TokenType::identifier, "int", lexer::Position(20, 20)},
    {lexer::TokenType::rparen, ")", lexer::Position(20, 23)},
    {lexer::TokenType::equal_op, "=", lexer::Position(20, 25)},

    {lexer::TokenType::lparen, "(", lexer::Position(21, 1)},

    {lexer::TokenType::if_keyword, "if", lexer::Position(22, 6)},
    {lexer::TokenType::identifier, "c", lexer::Position(22, 9)},
    {lexer::TokenType::equal_op, "=", lexer::Position(22, 10)},
    {lexer::TokenType::identifier, "N", lexer::Position(22, 11)},

    {lexer::TokenType::then_keyword, "then", lexer::Position(23, 6)},
    {lexer::TokenType::identifier, "printboard", lexer::Position(23, 11)},
    {lexer::TokenType::lparen, "(", lexer::Position(23, 21)},
    {lexer::TokenType::rparen, ")", lexer::Position(23, 22)},

    {lexer::TokenType::else_keyword, "else", lexer::Position(24, 6)},
    {lexer::TokenType::for_keyword, "for", lexer::Position(24, 11)},
    {lexer::TokenType::identifier, "r", lexer::Position(24, 15)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(24, 17)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(24, 20)},
    {lexer::TokenType::to_keyword, "to", lexer::Position(24, 22)},
    {lexer::TokenType::identifier, "N", lexer::Position(24, 25)},
    {lexer::TokenType::minus_op, "-", lexer::Position(24, 26)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(24, 27)},

    {lexer::TokenType::do_keyword, "do", lexer::Position(25, 9)},
    {lexer::TokenType::if_keyword, "if", lexer::Position(25, 12)},
    {lexer::TokenType::identifier, "row", lexer::Position(25, 15)},
    {lexer::TokenType::lbracket, "[", lexer::Position(25, 18)},
    {lexer::TokenType::identifier, "r", lexer::Position(25, 19)},
    {lexer::TokenType::rbracket, "]", lexer::Position(25, 20)},
    {lexer::TokenType::equal_op, "=", lexer::Position(25, 21)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(25, 22)},
    {lexer::TokenType::and_op, "&", lexer::Position(25, 24)},
    {lexer::TokenType::identifier, "diag1", lexer::Position(25, 26)},
    {lexer::TokenType::lbracket, "[", lexer::Position(25, 31)},
    {lexer::TokenType::identifier, "r", lexer::Position(25, 32)},
    {lexer::TokenType::plus_op, "+", lexer::Position(25, 33)},
    {lexer::TokenType::identifier, "c", lexer::Position(25, 34)},
    {lexer::TokenType::rbracket, "]", lexer::Position(25, 35)},
    {lexer::TokenType::equal_op, "=", lexer::Position(25, 36)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(25, 37)},
    {lexer::TokenType::and_op, "&", lexer::Position(25, 39)},
    {lexer::TokenType::identifier, "diag2", lexer::Position(25, 41)},
    {lexer::TokenType::lbracket, "[", lexer::Position(25, 46)},
    {lexer::TokenType::identifier, "r", lexer::Position(25, 47)},
    {lexer::TokenType::plus_op, "+", lexer::Position(25, 48)},
    {lexer::TokenType::integer_literal, "7", lexer::Position(25, 49)},
    {lexer::TokenType::minus_op, "-", lexer::Position(25, 50)},
    {lexer::TokenType::identifier, "c", lexer::Position(25, 51)},
    {lexer::TokenType::rbracket, "]", lexer::Position(25, 52)},
    {lexer::TokenType::equal_op, "=", lexer::Position(25, 53)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(25, 54)},

    {lexer::TokenType::then_keyword, "then", lexer::Position(26, 17)},
    {lexer::TokenType::lparen, "(", lexer::Position(26, 22)},
    {lexer::TokenType::identifier, "row", lexer::Position(26, 23)},
    {lexer::TokenType::lbracket, "[", lexer::Position(26, 26)},
    {lexer::TokenType::identifier, "r", lexer::Position(26, 27)},
    {lexer::TokenType::rbracket, "]", lexer::Position(26, 28)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(26, 29)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(26, 31)},
    {lexer::TokenType::semicolon, ";", lexer::Position(26, 32)},
    {lexer::TokenType::identifier, "diag1", lexer::Position(26, 34)},
    {lexer::TokenType::lbracket, "[", lexer::Position(26, 39)},
    {lexer::TokenType::identifier, "r", lexer::Position(26, 40)},
    {lexer::TokenType::plus_op, "+", lexer::Position(26, 41)},
    {lexer::TokenType::identifier, "c", lexer::Position(26, 42)},
    {lexer::TokenType::rbracket, "]", lexer::Position(26, 43)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(26, 44)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(26, 46)},
    {lexer::TokenType::semicolon, ";", lexer::Position(26, 47)},
    {lexer::TokenType::identifier, "diag2", lexer::Position(26, 49)},
    {lexer::TokenType::lbracket, "[", lexer::Position(26, 54)},
    {lexer::TokenType::identifier, "r", lexer::Position(26, 55)},
    {lexer::TokenType::plus_op, "+", lexer::Position(26, 56)},
    {lexer::TokenType::integer_literal, "7", lexer::Position(26, 57)},
    {lexer::TokenType::minus_op, "-", lexer::Position(26, 58)},
    {lexer::TokenType::identifier, "c", lexer::Position(26, 59)},
    {lexer::TokenType::rbracket, "]", lexer::Position(26, 60)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(26, 61)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(26, 63)},
    {lexer::TokenType::semicolon, ";", lexer::Position(26, 64)},

    {lexer::TokenType::identifier, "col", lexer::Position(27, 20)},
    {lexer::TokenType::lbracket, "[", lexer::Position(27, 23)},
    {lexer::TokenType::identifier, "c", lexer::Position(27, 24)},
    {lexer::TokenType::rbracket, "]", lexer::Position(27, 25)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(27, 26)},
    {lexer::TokenType::identifier, "r", lexer::Position(27, 28)},
    {lexer::TokenType::semicolon, ";", lexer::Position(27, 29)},

    {lexer::TokenType::identifier, "try", lexer::Position(28, 23)},
    {lexer::TokenType::lparen, "(", lexer::Position(28, 26)},
    {lexer::TokenType::identifier, "c", lexer::Position(28, 27)},
    {lexer::TokenType::plus_op, "+", lexer::Position(28, 28)},
    {lexer::TokenType::integer_literal, "1", lexer::Position(28, 29)},
    {lexer::TokenType::rparen, ")", lexer::Position(28, 30)},
    {lexer::TokenType::semicolon, ";", lexer::Position(28, 31)},

    {lexer::TokenType::identifier, "row", lexer::Position(29, 17)},
    {lexer::TokenType::lbracket, "[", lexer::Position(29, 20)},
    {lexer::TokenType::identifier, "r", lexer::Position(29, 21)},
    {lexer::TokenType::rbracket, "]", lexer::Position(29, 22)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(29, 23)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(29, 25)},
    {lexer::TokenType::semicolon, ";", lexer::Position(29, 26)},
    {lexer::TokenType::identifier, "diag1", lexer::Position(29, 28)},
    {lexer::TokenType::lbracket, "[", lexer::Position(29, 33)},
    {lexer::TokenType::identifier, "r", lexer::Position(29, 34)},
    {lexer::TokenType::plus_op, "+", lexer::Position(29, 35)},
    {lexer::TokenType::identifier, "c", lexer::Position(29, 36)},
    {lexer::TokenType::rbracket, "]", lexer::Position(29, 37)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(29, 38)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(29, 40)},
    {lexer::TokenType::semicolon, ";", lexer::Position(29, 41)},
    {lexer::TokenType::identifier, "diag2", lexer::Position(29, 43)},
    {lexer::TokenType::lbracket, "[", lexer::Position(29, 48)},
    {lexer::TokenType::identifier, "r", lexer::Position(29, 49)},
    {lexer::TokenType::plus_op, "+", lexer::Position(29, 50)},
    {lexer::TokenType::integer_literal, "7", lexer::Position(29, 51)},
    {lexer::TokenType::minus_op, "-", lexer::Position(29, 52)},
    {lexer::TokenType::identifier, "c", lexer::Position(29, 53)},
    {lexer::TokenType::rbracket, "]", lexer::Position(29, 54)},
    {lexer::TokenType::assign_op, ":=", lexer::Position(29, 55)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(29, 57)},
    {lexer::TokenType::rparen, ")", lexer::Position(29, 58)},

    {lexer::TokenType::rparen, ")", lexer::Position(31, 1)},

    {lexer::TokenType::in_keyword, "in", lexer::Position(32, 2)},
    {lexer::TokenType::identifier, "try", lexer::Position(32, 5)},
    {lexer::TokenType::lparen, "(", lexer::Position(32, 8)},
    {lexer::TokenType::integer_literal, "0", lexer::Position(32, 9)},
    {lexer::TokenType::rparen, ")", lexer::Position(32, 10)},

    {lexer::TokenType::end_keyword, "end", lexer::Position(33, 1)}};
    // clang-format off

  CHECK_NOTHROW(for(auto t : tokens) { CHECK(scanner.next() == t); };);
  CHECK_NOTHROW(CHECK(scanner.next().type == lexer::TokenType::eof));
}

TEST_CASE("multiline-string")
{
  // clang-format off
  std::string s = std::string("var s : string = \"hello \\n\\\n") + 
  "world\\\n" + 
  "!!!\"";
  // clang-format on
  lexer::Scanner scanner(s);

  CHECK_NOTHROW(CHECK((
    // clang-format off
    scanner.next() == lexer::Token(lexer::TokenType::var_keyword, "var", lexer::Position(1, 1)) &&
    scanner.next() == lexer::Token(lexer::TokenType::identifier, "s", lexer::Position(1, 5)) &&
    scanner.next() == lexer::Token(lexer::TokenType::colon, ":", lexer::Position(1, 7)) &&
    scanner.next() == lexer::Token(lexer::TokenType::identifier, "string", lexer::Position(1, 9)) &&
    scanner.next() == lexer::Token(lexer::TokenType::equal_op, "=", lexer::Position(1, 16)) &&
    scanner.next() == lexer::Token(lexer::TokenType::string_literal, "hello \nworld!!!", lexer::Position(1, 18)))););
  // clang-format on
}

TEST_SUITE_END();