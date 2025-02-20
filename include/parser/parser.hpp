#pragma once
#include <deque>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>

namespace parser
{

class Parser {
  public:
  Parser(lexer::Scanner& scanner)
    : scanner(scanner){};
  void parse();

  private:
  bool match(const lexer::Token& tok);
  void expect(lexer::TokenType type, const std::string& err_msg);
  lexer::Token next();
  lexer::Token peek(int distance = 0);
  void error_at(const lexer::Token& tok, const std::string& err_msg);
  std::deque<lexer::Token> tokens;
  lexer::Scanner& scanner;
  std::shared_ptr<ast::Expression> expression();
  std::shared_ptr<ast::SeqExp> sequencing();
};

} // namespace parser