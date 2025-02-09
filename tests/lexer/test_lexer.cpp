#include <doctest/doctest.h>
#include <lexer/lex.hpp>

TEST_SUITE_BEGIN("lexer");

TEST_CASE("valid")
{

	std::string test = "ciao 2";
	lexer::Scanner scanner(test);

	CHECK(scanner.next() ==
				lexer::Token{lexer::TokenType::identifier, "ciao"});
}

TEST_SUITE_END();