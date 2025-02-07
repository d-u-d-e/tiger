#pragma once
#include "token.hpp"
#include <assert.h>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace lexer
{

class Scanner {
public:
	Scanner(const std::string& filename)
	{
		auto f = std::ifstream(filename);

		if(!f.is_open()) {
			std::cerr << "Failed to open the file." << std::endl;
			exit(1);
		}

		std::stringstream buffer;
		buffer << f.rdbuf();
		contents = std::move(buffer.str());
		current = contents.c_str();
	}

	Token next()
	{
		skip_spaces();
		return read_token();
	}

private:
	bool is_eof(const char* current)
	{
		return current >= contents.c_str() + contents.size();
	}

	void skip_spaces()
	{
		while(!is_eof(current) && std::isspace(*current)) {
			current++;
		}
	}

	Token eof_token()
	{
		return {TokenType::eof, ""};
	}

	Token read_token()
	{
		if(is_eof(current)) {
			return eof_token();
		}
		else if(std::isalpha(*current)) {
			return identifier();
		}
		else if(std::isdigit(*current)) {
			return integer_literal();
		}
		else if(*current == '"') {
			return string_literal();
		}
		return punctuation();
	}

	Token identifier()
	{
		const char* start = current;
		while(!is_eof(current) && (std::isalnum(*current) || *current == '_')) {
			current++;
		}

		std::string value(start, current);
		if(keywords.find(value) != keywords.end()) {
			return {keywords.at(value), value};
		}
		return {TokenType::identifier, value};
	}

	Token string_literal()
	{
		// First token is the opening quote
		current++;
		std::string value;
		while(!is_eof(current)) {
			if(*current == '\\') {
				// multiline string
				if(std::isspace(peek(1))) {
					current++;
					skip_spaces();
					if(*current == '\\') {
						current++;
						continue;
					}
					else {
						std::cerr << "Unterminated string literal" << std::endl;
						exit(1);
					}
				}

				char ch = escape_sequence(&current);
				value += ch;
			}
			else if(*current != '"') {
				value += *current;
				current++;
			}
			else {
				current++; // closing quote
				return Token{TokenType::string_literal, value};
			}
		}

		std::cerr << "Unterminated string literal" << std::endl;
		exit(1);
	}

	Token integer_literal()
	{
		const char* start = current;
		while(!is_eof(current) && std::isdigit(*current)) {
			current++;
		}
		return {TokenType::integer_literal, std::string(start, current)};
	}

	Token punctuation()
	{
		// TODO
		return eof_token();
	}

	char peek(int offset = 0)
	{
		if(current + offset >= contents.c_str() + contents.size()) {
			return '\0';
		}
		return *(current + offset);
	}

	char escape_sequence(const char** current)
	{
		// current points to the backslash
		char ch1 = peek(1);
		char ch2 = peek(2);
		char ch3 = peek(3);

		// 3-digit octal
		if(std::isdigit(ch1) && std::isdigit(ch2) && std::isdigit(ch3)) {
			int v = 0;
			v = (ch1 - '0') * 64 + (ch2 - '0') * 8 + (ch3 - '0');
			if(v > 255) {
				std::cerr << "Invalid escape sequence" << std::endl;
				exit(1);
			}
			*current += 4;
			return (char)v;
		}

		switch(ch1) {
		case '"':
			*current += 2;
			return '"';
		case '\\':
			*current += 2;
			return '\\';
		case 'n':
			*current += 2;
			return '\n';
		case 't':
			*current += 2;
			return '\t';
		default:
			std::cerr << "Invalid escape sequence" << std::endl;
			exit(1);
		}
	}

	std::string contents;
	const char* current;
};

} // namespace lexer