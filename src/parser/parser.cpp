
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>
#include <utility>

namespace parser
{

void Parser::parse()
{
  current = scanner.next();
  pratt_table.insert({
    // clang-format off
    {lexer::TokenType::identifier,        PrecedenceRule(Precedence(None),        [this](){return variable();},         nullptr)},
    {lexer::TokenType::integer_literal,   PrecedenceRule(Precedence(None),        [this](){return integer_literal();},  nullptr)},
    {lexer::TokenType::string_literal,    PrecedenceRule(Precedence(None),        [this](){return string_literal();},   nullptr)},
    {lexer::TokenType::while_keyword,     PrecedenceRule(Precedence(None),        [this](){return while_expr();},       nullptr)},
    {lexer::TokenType::for_keyword,       PrecedenceRule(Precedence(None),        [this](){return for_expr();},         nullptr)},
    {lexer::TokenType::break_keyword,     PrecedenceRule(Precedence(None),        [this](){return break_expr();},       nullptr)},
    {lexer::TokenType::let_keyword,       PrecedenceRule(Precedence(None),        [this](){return let_expr();},         nullptr)},
    {lexer::TokenType::if_keyword,        PrecedenceRule(Precedence(None),        [this](){return if_expr();},          nullptr)},
    {lexer::TokenType::nil_keyword,       PrecedenceRule(Precedence(None),        [this](){return nil_literal();},      nullptr)},
    {lexer::TokenType::lparen,            PrecedenceRule(Precedence(Call),        [this](){return sequencing();},       std::bind(&Parser::call_expr, this, std::placeholders::_1))},
    {lexer::TokenType::lbracket,          PrecedenceRule(Precedence(Call),        nullptr,                              std::bind(&Parser::array_subscript, this, std::placeholders::_1))},
    {lexer::TokenType::lbrace,            PrecedenceRule(Precedence(Call),        nullptr,                              std::bind(&Parser::record_expr, this, std::placeholders::_1))},
    {lexer::TokenType::dot_op,            PrecedenceRule(Precedence(Call),        nullptr,                              std::bind(&Parser::record_field, this, std::placeholders::_1))},
    {lexer::TokenType::plus_op,           PrecedenceRule(Precedence(Term),        nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::minus_op,          PrecedenceRule(Precedence(Term),        [this](){return unary_expr();},       std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::times_op,          PrecedenceRule(Precedence(Factor),      nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::divide_op,         PrecedenceRule(Precedence(Factor),      nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::equal_op,          PrecedenceRule(Precedence(Comparison),  nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::not_equal_op,      PrecedenceRule(Precedence(Comparison),  nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::less_op,           PrecedenceRule(Precedence(Comparison),  nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::less_equal_op,     PrecedenceRule(Precedence(Comparison),  nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::greater_op,        PrecedenceRule(Precedence(Comparison),  nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::greater_equal_op,  PrecedenceRule(Precedence(Comparison),  nullptr,                              std::bind(&Parser::binary_expr, this, std::placeholders::_1))},
    {lexer::TokenType::and_op,            PrecedenceRule(Precedence(And),         nullptr,                              std::bind(&Parser::and_expr, this, std::placeholders::_1))},
    {lexer::TokenType::or_op,             PrecedenceRule(Precedence(Or),          nullptr,                              std::bind(&Parser::or_expr, this, std::placeholders::_1))},
    {lexer::TokenType::assign_op,         PrecedenceRule(Precedence(Assignment),  nullptr,                              std::bind(&Parser::assign_expr, this, std::placeholders::_1))},
    
    {lexer::TokenType::eof,               PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::to_keyword,        PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::in_keyword,        PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::end_keyword,       PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::function_keyword,  PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::var_keyword,       PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::type_keyword,      PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::array_keyword,     PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::then_keyword,      PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::else_keyword,      PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::do_keyword,        PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::of_keyword,        PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::comma,             PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::colon,             PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::semicolon,         PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::rparen,            PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::rbracket,          PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    {lexer::TokenType::rbrace,            PrecedenceRule(Precedence(None),        nullptr,                              nullptr)},
    // clang-format on
  });
  auto exp = expression(Precedence::None);
  // do something with it
  ASTVisitor pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl;
}

std::shared_ptr<ast::Expression> Parser::expression(int precedence)
{

  std::shared_ptr<ast::Expression> lhs;
  advance();
  auto rule = pratt_table.at(previous.type);
  if(rule.prefix_rule == nullptr) {
    error_at(previous, "Expected expression");
    return nullptr;
  }
  // when the prefix rule gets called, the prefix token has already been consumed
  lhs = rule.prefix_rule();

  rule = pratt_table.at(current.type);

  while(rule.precedence_value > precedence) {
    if(rule.infix_rule == nullptr) {
      error_at(current, "Unexpected token");
      return nullptr;
    }
    advance(); // skip infix operator
    lhs = rule.infix_rule(lhs);
    rule = pratt_table.at(current.type);
  }
  return lhs;
}

std::shared_ptr<ast::SeqExp> Parser::sequencing()
{
  std::vector<std::pair<std::shared_ptr<ast::Expression>, int>> exps;
  // rule: '(' ')'
  if(match(lexer::TokenType::rparen)) {
    return std::make_shared<ast::SeqExp>(exps);
  }

  // rule: <exp> (';' <exp>)* ')'
  do {
    auto pos = current.pos;
    exps.push_back({expression(Precedence::None), pos});
  } while(match(lexer::TokenType::semicolon));

  expect(lexer::TokenType::rparen, "Expected ')'");
  return std::make_shared<ast::SeqExp>(exps);
}

std::shared_ptr<ast::VarExp> Parser::variable()
{
  auto var =
    std::make_shared<ast::SimpleVar>(Symbol{previous.value}, previous.pos);
  return std::make_shared<ast::VarExp>(var);
}

std::shared_ptr<ast::VarExp>
Parser::record_field(std::shared_ptr<ast::Expression> lhs)
{
  auto lhs_var = std::dynamic_pointer_cast<ast::VarExp>(lhs);

  if(!lhs_var) {
    error_at(previous, "Expected variable");
    return nullptr;
  }

  expect(lexer::TokenType::identifier, "Expected record field name");
  auto field = previous;
  auto var = std::make_shared<ast::FieldVar>(
    lhs_var->var, Symbol(field.value), field.pos);

  return std::make_shared<ast::VarExp>(var);
}

std::shared_ptr<ast::Expression>
Parser::array_subscript(std::shared_ptr<ast::Expression> lhs)
{
  auto subscript_tok = previous;
  // Parse the expression between brackets
  auto between_exp = expression(Precedence::None);
  expect(lexer::TokenType::rbracket, "Expected ']'");

  // Next check whether we have an array exp or a subscript var
  auto lhs_var = std::dynamic_pointer_cast<ast::VarExp>(lhs);

  if(match(lexer::TokenType::of_keyword)) {
    // The parser must have found a simple variable as lhs
    if(!lhs_var || typeid(*lhs_var->var) != typeid(ast::SimpleVar)) {
      error_at(subscript_tok, "Invalid array expression");
      return nullptr;
    }

    auto simple_var = std::dynamic_pointer_cast<ast::SimpleVar>(lhs_var->var);
    auto ty_symbol = simple_var->name;

    // Compute the init expression
    std::shared_ptr<ast::Expression> init = expression(Precedence::None);

    return std::make_shared<ast::ArrayExp>(
      ty_symbol, between_exp, init, simple_var->position);
  }

  // Otherwise this must be a subscript var
  if(!lhs_var) {
    error_at(subscript_tok, "Only variables are subscriptable");
    return nullptr;
  }

  auto var = std::make_shared<ast::SubscriptVar>(
    lhs_var->var, between_exp, subscript_tok.pos);
  return std::make_shared<ast::VarExp>(var);
}

std::shared_ptr<ast::IntExp> Parser::integer_literal()
{
  return std::make_shared<ast::IntExp>(std::stoi(previous.value));
}

std::shared_ptr<ast::StringExp> Parser::string_literal()
{
  return std::make_shared<ast::StringExp>(previous.value, previous.pos);
}

std::shared_ptr<ast::WhileExp> Parser::while_expr()
{
  // TODO
  //std::cout << "while_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::ForExp> Parser::for_expr()
{
  // TODO
  //std::cout << "for_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::BreakExp> Parser::break_expr()
{
  // TODO
  //std::cout << "break_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::LetExp> Parser::let_expr()
{
  // TODO
  //std::cout << "let_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::IfExp> Parser::if_expr()
{
  // TODO
  //std::cout << "if_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::NilExp> Parser::nil_literal()
{
  return std::make_shared<ast::NilExp>();
}

std::shared_ptr<ast::OpExp>
Parser::binary_expr(std::shared_ptr<ast::Expression> lhs)
{
  //std::cout << "binary_expr" << std::endl;

  auto op = previous;
  auto prec = pratt_table.at(op.type).precedence_value;
  auto rhs = expression(prec + 1);
  return std::make_shared<ast::OpExp>(lhs, map_operator(op.type), rhs, op.pos);
}

std::shared_ptr<ast::OpExp> Parser::unary_expr()
{
  // TODO
  //std::cout << "unary_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::OpExp>
Parser::and_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  //std::cout << "and_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::OpExp>
Parser::or_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  //std::cout << "or_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::AssignExp>
Parser::assign_expr(std::shared_ptr<ast::Expression> lhs)
{
  //std::cout << "assign_expr" << std::endl;
  auto op = previous;
  auto rhs = expression(Precedence::Assignment);

  auto var = std::dynamic_pointer_cast<ast::VarExp>(lhs);

  if(var) {
    return std::make_shared<ast::AssignExp>(var->var, rhs, op.pos);
  }

  error_at(op, "Invalid assignment target");
  return nullptr;
}

std::shared_ptr<ast::CallExp>
Parser::call_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  //std::cout << "call_expr" << std::endl;
  return nullptr;
}

std::shared_ptr<ast::Expression>
Parser::record_expr(std::shared_ptr<ast::Expression> lhs)
{
  // TODO
  //std::cout << "record_expr" << std::endl;
  return nullptr;
}

void Parser::advance()
{
  //std::cout << current << std::endl;
  previous = current;
  current = scanner.next();
}

bool Parser::check(lexer::TokenType type)
{
  return current.type == type;
}

bool Parser::match(lexer::TokenType type)
{
  if(check(type)) {
    advance();
    return true;
  }
  return false;
}

void Parser::expect(lexer::TokenType type, const std::string& err_msg)
{
  if(check(type)) {
    advance();
    return;
  }
  error_at(current, err_msg);
}

void Parser::error_at(const lexer::Token& tok, const std::string& err_msg)
{
  throw std::runtime_error(std::format(
    "[line {}:{}] Err at {}: {}\n", tok.line, tok.pos, tok, err_msg));
}

ast::Operator Parser::map_operator(lexer::TokenType type)
{
  switch(type) {
  case lexer::TokenType::plus_op:
    return ast::Operator::Plus;
  case lexer::TokenType::minus_op:
    return ast::Operator::Minus;
  case lexer::TokenType::times_op:
    return ast::Operator::Times;
  case lexer::TokenType::divide_op:
    return ast::Operator::Divide;
  case lexer::TokenType::equal_op:
    return ast::Operator::Equal;
  case lexer::TokenType::not_equal_op:
    return ast::Operator::NotEqual;
  case lexer::TokenType::less_op:
    return ast::Operator::Less;
  case lexer::TokenType::less_equal_op:
    return ast::Operator::LessEqual;
  case lexer::TokenType::greater_op:
    return ast::Operator::Greater;
  case lexer::TokenType::greater_equal_op:
    return ast::Operator::GreaterEqual;
  }
  assert(false);
  std::unreachable();
}

} // namespace parser