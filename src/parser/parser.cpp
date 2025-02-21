
#include <parser/parser.hpp>

namespace parser
{

void Parser::parse()
{

  pratt_table.insert({
    // clang-format off
    {lexer::TokenType::identifier,        PrecedenceRule(Precedence(Primary),     [this](){return simple_var();},      nullptr)},
    {lexer::TokenType::integer_literal,   PrecedenceRule(Precedence(Primary),     [this](){return integer_literal();}, nullptr)},
    {lexer::TokenType::string_literal,    PrecedenceRule(Precedence(Primary),     [this](){return string_literal();},  nullptr)},
    {lexer::TokenType::while_keyword,     PrecedenceRule(Precedence(None),        [this](){return while_expr();},      nullptr)},
    {lexer::TokenType::for_keyword,       PrecedenceRule(Precedence(None),        [this](){return for_expr();},        nullptr)},
    {lexer::TokenType::break_keyword,     PrecedenceRule(Precedence(None),        [this](){return break_expr();},      nullptr)},
    {lexer::TokenType::let_keyword,       PrecedenceRule(Precedence(None),        [this](){return let_expr();},        nullptr)},
    {lexer::TokenType::if_keyword,        PrecedenceRule(Precedence(None),        [this](){return if_expr();},         nullptr)},
    {lexer::TokenType::nil_keyword,       PrecedenceRule(Precedence(Primary),     [this](){return nil_literal();},     nullptr)},
    {lexer::TokenType::lparen,            PrecedenceRule(Precedence(Call),        [this](){return sequencing();},      std::bind(&Parser::call_expr, this, std::placeholders::_1))},
    {lexer::TokenType::lbracket,          PrecedenceRule(Precedence(Call),        nullptr,                             std::bind(&Parser::array_subscript, this, std::placeholders::_1))},
    {lexer::TokenType::lbrace,            PrecedenceRule(Precedence(Call),        nullptr,                             std::bind(&Parser::record_expr, this, std::placeholders::_1))},
    {lexer::TokenType::dot_op,            PrecedenceRule(Precedence(Call),        nullptr,                             std::bind(&Parser::field_var, this, std::placeholders::_1))},
    {lexer::TokenType::plus_op,           PrecedenceRule(Precedence(Term),        nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::minus_op,          PrecedenceRule(Precedence(Term),        [this](){return unary_expr();},      std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::times_op,          PrecedenceRule(Precedence(Factor),      nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::divide_op,         PrecedenceRule(Precedence(Factor),      nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::equal_op,          PrecedenceRule(Precedence(Comparison),  nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::not_equal_op,      PrecedenceRule(Precedence(Comparison),  nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::less_op,           PrecedenceRule(Precedence(Comparison),  nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::less_equal_op,     PrecedenceRule(Precedence(Comparison),  nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::greater_op,        PrecedenceRule(Precedence(Comparison),  nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::greater_equal_op,  PrecedenceRule(Precedence(Comparison),  nullptr,                             std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::and_op,            PrecedenceRule(Precedence(And),         nullptr,                             std::bind(&Parser::and_expr, this, std::placeholders::_1))},
    {lexer::TokenType::or_op,             PrecedenceRule(Precedence(Or),          nullptr,                             std::bind(&Parser::or_expr, this, std::placeholders::_1))},
    {lexer::TokenType::assign_op,         PrecedenceRule(Precedence(Assignment),  nullptr,                             std::bind(&Parser::assign_expr, this, std::placeholders::_1))},
    // clang-format on
  });
  auto exp = expression(Precedence::None);
  // do something with it
}

std::shared_ptr<ast::Expression> Parser::expression(int precedence)
{
  auto current = next();
  std::shared_ptr<ast::Expression> lhs;
  try {
    auto rule = pratt_table.at(current.type);
    if(rule.prefix_rule == nullptr) {
      error_at(current, "Expected expression");
      return nullptr;
    }
    lhs = rule.prefix_rule();

    while(rule.precedence_value > precedence) {
      auto op = current;
      current = next();
      if(rule.infix_rule == nullptr) {
        break;
      }
      lhs = rule.infix_rule(lhs);
      rule = pratt_table.at(current.type);
    }
  }
  catch(std::out_of_range& e) {
    error_at(current, "Unexpected token");
    return nullptr;
  }
  return lhs;
}

std::shared_ptr<ast::SeqExp> Parser::sequencing()
{
  std::cout << "sequencing" << std::endl;

  std::vector<std::shared_ptr<ast::Expression>> exps;
  // rule: '(' ')'
  if(peek(0).type == lexer::TokenType::rparen) {
    return std::make_shared<ast::SeqExp>(exps);
  }

  // rule: <exp> (';' <exp>)* ')'
  do {
    exps.push_back(expression(Precedence::None));
  } while(peek(0).type == lexer::TokenType::semicolon);

  return std::make_shared<ast::SeqExp>(exps);
}

std::shared_ptr<ast::VarExp> Parser::simple_var()
{
  // TODO
  std::cout << "simple_var" << std::endl;
  return nullptr;
}
std::shared_ptr<ast::VarExp>
Parser::field_var(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  std::cout << "field_var" << std::endl;
  return nullptr;
}
std::shared_ptr<ast::Expression>
Parser::array_subscript(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  std::cout << "array_subscript" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::IntExp> Parser::integer_literal()
{
  // TODO
  std::cout << "integer_literal" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::StringExp> Parser::string_literal()
{
  // TODO
  std::cout << "string_literal" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::WhileExp> Parser::while_expr()
{
  // TODO
  std::cout << "while_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::ForExp> Parser::for_expr()
{
  // TODO
  std::cout << "for_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::BreakExp> Parser::break_expr()
{
  // TODO
  std::cout << "break_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::LetExp> Parser::let_expr()
{
  // TODO
  std::cout << "let_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::IfExp> Parser::if_expr()
{
  // TODO
  std::cout << "if_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::NilExp> Parser::nil_literal()
{
  // TODO
  std::cout << "nil_literal" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::OpExp>
Parser::binary_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  std::cout << "binary_expr" << std::endl;
  return nullptr;
}
std::shared_ptr<ast::OpExp> Parser::unary_expr()
{
  // TODO
  std::cout << "unary_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::OpExp>
Parser::and_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  std::cout << "and_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::OpExp>
Parser::or_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  std::cout << "or_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::AssignExp>
Parser::assign_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  std::cout << "assign_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::CallExp>
Parser::call_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  std::cout << "call_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::Expression>
Parser::record_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  std::cout << "record_expr" << std::endl;
  return nullptr;
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