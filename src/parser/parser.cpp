#include <parser/parser.hpp>

namespace parser
{

void Parser::parse()
{

}

std::shared_ptr<ast::Expression> Parser::expression()
{
  auto token = next();
  error_at(token, "Unexpected token");
  return nullptr;
}

std::shared_ptr<ast::SeqExp> Parser::sequencing()
{
  std::vector<std::shared_ptr<ast::Expression>> exps;
  
  // rule: '(' ')'
  if (peek(0).type == lexer::TokenType::rparen) {
    return std::make_shared<ast::SeqExp>(exps);
  }

  // rule: <exp> (';' <exp>)* ')'
  do {
    exps.push_back(expression());
  } while (peek(0).type == lexer::TokenType::semicolon);

  return std::make_shared<ast::SeqExp>(exps);
}

lexer::Token Parser::next()
{
  if(tokens.empty()) {
    tokens.push_back(scanner.next());
  }
  auto token = tokens.front();
  tokens.pop_front();
  return token;
}

lexer::Token Parser::peek(int distance)
{
  while(tokens.size() <= distance) {
    tokens.push_back(scanner.next());
  }
  return tokens.at(distance);
}

bool Parser::match(const lexer::Token& tok)
{
  auto peeked = peek(0);
  if(peeked.type == tok.type && peeked.value == tok.value) {
    tokens.pop_front();
    return true;
  }
  return false;
}

void Parser::expect(lexer::TokenType type, const std::string& err_msg)
{
  auto peeked = peek(0);
  if(peeked.type == type) {
    tokens.pop_front();
    return;
  }
  error_at(peeked, err_msg);
}

void Parser::error_at(const lexer::Token& tok, const std::string& err_msg)
{
  throw std::runtime_error(
    std::format("[line {}] Err: {}\n", tok.line, err_msg));
}

} // namespace parser