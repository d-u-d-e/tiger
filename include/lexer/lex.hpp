#pragma once
#include "token.hpp"
#include <assert.h>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace lexer
{

class Scanner {
	public:
	Scanner(const std::filesystem::path& filename)
	{
		auto f = std::ifstream(filename);

		if(!f.is_open()) {
			error("Could not open file");
		}

		std::stringstream buffer;
		buffer << f.rdbuf();
		contents = std::move(buffer.str());
		current = contents.c_str();
		line = 1;
	}

	Scanner(const std::string& src)
	{
		contents = src;
		current = contents.c_str();
		line = 1;
	}

	Token next()
	{
		skip_spaces();
		return read_token();
	}

	private:
	Token read_token();
	Token identifier();
	Token string_literal();
	Token integer_literal();
	Token punctuation();
	void skip_multiline_comment();
	char escape_sequence(const char** current);

	char peek(int offset = 0)
	{
		if(current + offset >= contents.c_str() + contents.size()) {
			return '\0';
		}
		return *(current + offset);
	}

	void expect(char ch, const std::string& err_msg)
	{
		if(*current != ch) {
			error(err_msg);
		}
		current++;
	}

	bool match(char ch)
	{
		if(*current == ch) {
			current++;
			return true;
		}
		return false;
	}

	void error(const std::string& err_msg)
	{
		std::cerr << "[line " + std::to_string(line) + "] Err: " + err_msg
							<< std::endl;
		exit(1);
	}

	Token eof_token()
	{
		return {TokenType::eof, "$", line};
	}

	bool is_eof(const char* current)
	{
		return current >= (contents.c_str() + contents.size());
	}

	void skip_spaces()
	{
		while(!is_eof(current) && std::isspace(*current)) {
			if(*current == '\n') {
				line++;
			}
			current++;
		}
	}

	private:
	std::string contents;
	int line;
	const char* current;
};

} // namespace lexer