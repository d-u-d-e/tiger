
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

std::unique_ptr<ast::Expression> Parser::expression(int precedence)
{

  std::unique_ptr<ast::Expression> lhs;
  advance();
  auto rule = pratt_table.at(previous.type);
  if(rule.prefix_rule == nullptr) {
    error_at(previous, "expected expression");
    return nullptr;
  }
  // when the prefix rule gets called, the prefix token has already been consumed
  lhs = rule.prefix_rule();

  rule = pratt_table.at(current.type);

  while(rule.precedence_value > precedence) {
    if(rule.infix_rule == nullptr) {
      error_at(current, "unexpected token");
      return nullptr;
    }
    advance(); // skip infix operator
    lhs = rule.infix_rule(std::move(lhs));
    rule = pratt_table.at(current.type);
  }
  return lhs;
}

std::unique_ptr<ast::SeqExp> Parser::sequencing()
{
  // rule: '(' ')'
  // rule: '(' <exp> (';' <exp>)* ')'

  std::vector<std::pair<std::unique_ptr<ast::Expression>, lexer::Position>>
    exps{};
  if(match(lexer::TokenType::rparen)) {
    return std::make_unique<ast::SeqExp>(std::move(exps));
  }

  do {
    auto pos = current.pos;
    exps.emplace_back(expression(Precedence::None), pos);
  } while(match(lexer::TokenType::semicolon));

  expect(lexer::TokenType::rparen,
         "expected ')' closing a sequence expression");
  return std::make_unique<ast::SeqExp>(std::move(exps));
}

std::unique_ptr<ast::VarExp> Parser::variable()
{
  // we parsed an identifier as an infix operator
  auto var =
    std::make_unique<ast::SimpleVar>(Symbol{previous.value}, previous.pos);
  return std::make_unique<ast::VarExp>(std::move(var));
}

std::unique_ptr<ast::VarExp>
Parser::record_field(std::unique_ptr<ast::Expression> lhs)
{
  // rule: <id> '.' <id>
  auto lhs_var = dynamic_cast<ast::VarExp*>(lhs.get());

  if(!lhs_var) {
    error_at(previous, "expected variable before token '.'");
    return nullptr;
  }

  expect(lexer::TokenType::identifier,
         "expected record field name after token '.'");
  auto field = previous;
  auto var = std::make_unique<ast::FieldVar>(
    std::move(lhs_var->var), Symbol(field.value), field.pos);
  return std::make_unique<ast::VarExp>(std::move(var));
}

std::unique_ptr<ast::Expression>
Parser::array_subscript(std::unique_ptr<ast::Expression> lhs)
{
  // rule: <id> '[' <exp> ']'
  // rule: <id> '[' <exp> ']' of <exp>

  auto subscript_tok = previous;
  // parse the expression between brackets
  auto between_exp = expression(Precedence::None);
  expect(lexer::TokenType::rbracket,
         "expected ']' closing subscript expression");

  // next check whether we have an array exp or a subscript var
  auto lhs_var = dynamic_cast<ast::VarExp*>(lhs.get());

  if(!lhs_var) {
    error_at(subscript_tok, "expected variable before token '['");
    return nullptr;
  }

  if(match(lexer::TokenType::of_keyword)) {
    // the parser must have found a simple variable as lhs
    auto simple_var = dynamic_cast<ast::SimpleVar*>(lhs_var->var.get());

    if(!simple_var) {
      error_at(subscript_tok,
               "expected type identifier before token '[' of array expression");
      return nullptr;
    }

    auto ty_symbol = simple_var->name;

    // Compute the init expression
    std::unique_ptr<ast::Expression> init = expression(Precedence::None);

    return std::make_unique<ast::ArrayExp>(
      ty_symbol, std::move(between_exp), std::move(init), simple_var->position);
  }

  auto var = std::make_unique<ast::SubscriptVar>(
    std::move(lhs_var->var), std::move(between_exp), subscript_tok.pos);
  return std::make_unique<ast::VarExp>(std::move(var));
}

std::unique_ptr<ast::IntExp> Parser::integer_literal()
{
  return std::make_unique<ast::IntExp>(std::stoi(previous.value));
}

std::unique_ptr<ast::StringExp> Parser::string_literal()
{
  return std::make_unique<ast::StringExp>(previous.value, previous.pos);
}

std::unique_ptr<ast::WhileExp> Parser::while_expr()
{
  // rule: 'while' <exp> 'do' <exp>
  auto pos = previous.pos;
  auto cond = expression(Precedence::None);
  expect(lexer::TokenType::do_keyword, "expected 'do' after while condition");
  auto body = expression(Precedence::None);
  return std::make_unique<ast::WhileExp>(std::move(cond), std::move(body), pos);
}

std::unique_ptr<ast::ForExp> Parser::for_expr()
{
  // rule: 'for' <id> ':=' <exp> 'to' <exp> 'do' <exp>
  auto pos = previous.pos;
  expect(lexer::TokenType::identifier, "expected identifier");
  auto var = Symbol{previous.value};
  expect(lexer::TokenType::assign_op, "expected ':='");
  auto low = expression(Precedence::None);

  expect(lexer::TokenType::to_keyword, "expected 'to'");
  auto high = expression(Precedence::None);

  expect(lexer::TokenType::do_keyword, "expected 'do'");
  auto body = expression(Precedence::None);

  return std::make_unique<ast::ForExp>(
    var, std::move(low), std::move(high), std::move(body), pos);
}

std::unique_ptr<ast::BreakExp> Parser::break_expr()
{
  // rule: 'break'
  return std::make_unique<ast::BreakExp>(previous.pos);
}

std::unique_ptr<ast::LetExp> Parser::let_expr()
{
  // rule: 'let' <decls> 'in' <exp> 'end'
  auto pos = previous.pos;
  auto decs = decls();
  expect(lexer::TokenType::in_keyword, "expected 'in' after let decls");
  auto body = expression(Precedence::None);
  expect(lexer::TokenType::end_keyword, "expected 'end' after let expression");
  return std::make_unique<ast::LetExp>(std::move(decs), std::move(body), pos);
}

std::unique_ptr<ast::FuncDecl> Parser::func_decl()
{
  // rule: 'function' <id> '(' <tyfields> ')' '=' <exp>
  // rule: 'function' <id> '(' <tyfields> ')' ':' <id> '=' <exp>
  // rule: <tyfields> = epsilon
  // rule: <tyfields> = <id> ':' <id> (',' <id> ':' <id>)*

  // we have a vector because we might have mutually recursive functions
  std::vector<std::unique_ptr<ast::_FuncDecl>> fdecls;
  do {
    auto func_tok_pos = previous.pos;
    expect(lexer::TokenType::identifier, "expected function name");
    auto func_id = Symbol{previous.value};
    expect(lexer::TokenType::lparen, "expected '(' in function declaration");

    // parse params
    std::vector<ast::_Field> params;
    if(!check(lexer::TokenType::rparen)) {
      do {
        expect(lexer::TokenType::identifier, "expected parameter name");
        auto param_name = Symbol{previous.value};
        auto pos = previous.pos;
        expect(lexer::TokenType::colon, "expected ':' after parameter name");
        expect(lexer::TokenType::identifier, "expected parameter type");
        auto param_type = Symbol{previous.value};
        params.emplace_back(param_name, param_type, pos);
      } while(match(lexer::TokenType::comma));
    }
    expect(lexer::TokenType::rparen, "expected ')' in function declaration");

    // parse return type
    std::optional<Symbol> result;
    if(match(lexer::TokenType::colon)) {
      expect(lexer::TokenType::identifier, "expected function return type");
      result = Symbol{previous.value};
    }

    // parse body
    expect(lexer::TokenType::equal_op, "expected '=' before function body");
    auto body = expression(Precedence::None);
    auto fun_decl = std::make_unique<ast::_FuncDecl>(
      func_id, params, result, std::move(body), func_tok_pos);
    fdecls.emplace_back(std::move(fun_decl));

  } while(match(lexer::TokenType::function_keyword));

  return std::make_unique<ast::FuncDecl>(std::move(fdecls));
}

std::unique_ptr<ast::TypeDecl> Parser::type_decl()
{
  // rule: 'type' <id> '=' <ty>
  // rule: <ty> = <id> | '{' <tyfields> '}' | 'array' 'of' <id>
  // rule: <tyfields> = epsilon | <id> ':' <id> (',' <id> ':' <id>)*

  // we have a vector because we might have mutually recursive types
  std::vector<std::unique_ptr<ast::_TypeDecl>> decls_;

  do {
    auto pos = previous.pos;
    expect(lexer::TokenType::identifier,
           "expected type name after token 'type'");
    auto type_id = Symbol{previous.value};
    expect(lexer::TokenType::equal_op, "expected '=' after type identifier");

    if(match(lexer::TokenType::lbrace)) {
      // record type
      std::vector<ast::_Field> fields;
      if(!check(lexer::TokenType::rbrace)) {
        do {
          expect(lexer::TokenType::identifier, "expected parameter name");
          auto param_name = Symbol{previous.value};
          auto pos = previous.pos;
          expect(lexer::TokenType::colon, "expected ':' after parameter name");
          expect(lexer::TokenType::identifier,
                 "expected parameter type after token ':'");
          auto param_type = Symbol{previous.value};
          fields.emplace_back(param_name, param_type, pos);
        } while(match(lexer::TokenType::comma));
      }
      expect(lexer::TokenType::rbrace, "expected '}' after type fields");
      auto type = std::make_unique<ast::RecordType>(fields);
      decls_.emplace_back(
        std::make_unique<ast::_TypeDecl>(type_id, std::move(type), pos));
    }
    else if(match(lexer::TokenType::array_keyword)) {
      // array type
      expect(lexer::TokenType::of_keyword, "expected 'of' after 'array' token");
      expect(lexer::TokenType::identifier,
             "expected type identifier after 'of' token");
      auto type_sym = Symbol{previous.value};
      auto type = std::make_unique<ast::ArrayType>(type_sym, previous.pos);
      decls_.emplace_back(
        std::make_unique<ast::_TypeDecl>(type_id, std::move(type), pos));
    }
    else {
      // alias type
      expect(lexer::TokenType::identifier,
             "expected type identifier after '=' token");
      auto type_sym = Symbol{previous.value};
      auto type = std::make_unique<ast::NameType>(type_sym, previous.pos);
      decls_.emplace_back(
        std::make_unique<ast::_TypeDecl>(type_id, std::move(type), pos));
    }
  } while(match(lexer::TokenType::type_keyword));

  return std::make_unique<ast::TypeDecl>(std::move(decls_));
}

std::unique_ptr<ast::VarDecl> Parser::var_decl()
{

  // rule: 'var' <id> ':' <id> ':=' <exp>
  // rule: 'var' <id> ':=' <exp>

  auto pos = previous.pos;
  expect(lexer::TokenType::identifier, "expected variable identifier");
  auto var_id = Symbol{previous.value};
  std::optional<Symbol> var_type;
  if(match(lexer::TokenType::colon)) {
    expect(lexer::TokenType::identifier,
           "expected variable type after ':' token");
    var_type = Symbol{previous.value};
  }
  expect(lexer::TokenType::assign_op,
         "expected ':=' in a variable declaration");
  auto body = expression(Precedence::None);
  auto var_decl =
    std::make_unique<ast::VarDecl>(var_id, var_type, std::move(body), pos);
  return var_decl;
}

std::unique_ptr<ast::Declaration> Parser::decl()
{
  if(match(lexer::TokenType::var_keyword)) {
    return var_decl();
  }
  else if(match(lexer::TokenType::type_keyword)) {
    return type_decl();
  }
  else if(match(lexer::TokenType::function_keyword)) {
    return func_decl();
  }
  else {
    error_at(current, "expected declaration");
    return nullptr;
  }
}

std::vector<std::unique_ptr<ast::Declaration>> Parser::decls()
{
  // rule: <decl> (<decl>)*

  auto vec = std::vector<std::unique_ptr<ast::Declaration>>{};

  do {
    vec.emplace_back(decl());
  } while(check(lexer::TokenType::var_keyword) ||
          check(lexer::TokenType::type_keyword) ||
          check(lexer::TokenType::function_keyword));

  return vec;
}

std::unique_ptr<ast::IfExp> Parser::if_expr()
{
  // rule: 'if' <exp> 'then' <exp> ('else' <exp>)?

  auto pos = previous.pos;
  auto cond = expression(Precedence::None);
  expect(lexer::TokenType::then_keyword, "expected 'then' after if condition");
  auto then = expression(Precedence::None);

  std::unique_ptr<ast::Expression> else_ = nullptr;
  if(match(lexer::TokenType::else_keyword)) {
    // else belongs to closest if
    else_ = expression(Precedence::None);
  }
  return std::make_unique<ast::IfExp>(
    std::move(cond), std::move(then), std::move(else_), pos);
}

std::unique_ptr<ast::NilExp> Parser::nil_literal()
{
  return std::make_unique<ast::NilExp>();
}

std::unique_ptr<ast::OpExp>
Parser::binary_expr(std::unique_ptr<ast::Expression> lhs)
{
  auto op = previous;
  auto prec = pratt_table.at(op.type).precedence_value;
  auto rhs = expression(prec); // parse precedence > prec

  auto lhs_op = dynamic_cast<ast::OpExp*>(lhs.get());
  auto ast_op = map_operator(op.type);

  if(lhs_op && is_comparison_operator(ast_op) &&
     is_comparison_operator(lhs_op->op)) {
    // comparison is not associative
    error_at(op, "cannot chain comparison operators");
    return nullptr;
  }

  return std::make_unique<ast::OpExp>(
    std::move(lhs), ast_op, std::move(rhs), op.pos);
}

std::unique_ptr<ast::OpExp> Parser::unary_expr()
{
  // rule: '-' <exp>
  // This is implemented as a subtraction from 0 (Tiger only supports integer arithmetics)
  auto tok_pos = previous.pos;
  auto rhs = expression(Precedence::Unary);
  auto lhs = std::make_unique<ast::IntExp>(0);
  return std::make_unique<ast::OpExp>(
    std::move(lhs), ast::Operator::minus, std::move(rhs), tok_pos);
}

std::unique_ptr<ast::Expression>
Parser::and_expr(std::unique_ptr<ast::Expression> lhs)
{
  // rule: <exp> & <exp>
  // e1 & e2 is translated as `if e1 then e2 else 0`

  auto pos = previous.pos;
  auto rhs = expression(Precedence::And);
  return std::make_unique<ast::IfExp>(
    std::move(lhs), std::move(rhs), std::make_unique<ast::IntExp>(0), pos);
}

std::unique_ptr<ast::Expression>
Parser::or_expr(std::unique_ptr<ast::Expression> lhs)
{
  // rule: <exp> | <exp>
  // e1 | e2 is translated as `if e1 then e1 else e2`

  auto pos = previous.pos;
  auto rhs = expression(Precedence::Or);
  // we need shared pointers here :(
  auto shared_lhs = std::shared_ptr<ast::Expression>(std::move(lhs));
  return std::make_unique<ast::IfExp>(
    shared_lhs, shared_lhs, std::shared_ptr(std::move(rhs)), pos);
}

std::unique_ptr<ast::AssignExp>
Parser::assign_expr(std::unique_ptr<ast::Expression> lhs)
{
  // rule: <lvalue> ':=' <exp>
  // rule <lvalue> = <id> | <lvalue> '.' <id> | <lvalue> '[' <exp> ']'

  auto op = previous;
  auto rhs = expression(Precedence::Assignment);

  auto var = dynamic_cast<ast::VarExp*>(lhs.get());

  if(!var) {
    // asserting that lhs is an lvalue
    error_at(op, "invalid assignment target");
    return nullptr;
  }

  return std::make_unique<ast::AssignExp>(
    std::move(var->var), std::move(rhs), op.pos);
}

std::unique_ptr<ast::CallExp>
Parser::call_expr(std::unique_ptr<ast::Expression> lhs)
{
  // rule: <id> '(' <exp> (',' <exp>)* ')'

  auto pos = previous.pos;

  auto lhs_var = dynamic_cast<ast::VarExp*>(lhs.get());

  if(!lhs_var) {
    error_at(previous, "expected identifier as function name");
    return nullptr;
  }

  auto lhs_simple = dynamic_cast<ast::SimpleVar*>(lhs_var->var.get());

  if(!lhs_simple) {
    error_at(previous, "expected identifier as function name");
    return nullptr;
  }

  auto func_id = lhs_simple->name;
  std::vector<std::unique_ptr<ast::Expression>> args;

  if(!check(lexer::TokenType::rparen)) {
    do {
      args.emplace_back(expression(Precedence::None));
    } while(match(lexer::TokenType::comma));
  }

  expect(lexer::TokenType::rparen, "expected ')' after function arguments");
  return std::make_unique<ast::CallExp>(func_id, std::move(args), pos);
}

std::unique_ptr<ast::Expression>
Parser::record_expr(std::unique_ptr<ast::Expression> lhs)
{
  // rule: <id> '{' '}'
  // rule: <id> '{' <id> '=' <exp> (',' <id> '=' <exp>)*'}'

  auto lhs_var = dynamic_cast<ast::VarExp*>(lhs.get());
  if(!lhs_var) {
    error_at(previous, "expected identifier as record type");
    return nullptr;
  }

  auto lhs_simple = dynamic_cast<ast::SimpleVar*>(lhs_var->var.get());
  if(!lhs_simple) {
    error_at(previous, "expected identifier as record type");
    return nullptr;
  }

  auto type_sym = lhs_simple->name;
  auto type_pos = lhs_simple->position;
  std::vector<ast::_RecordField> fields;

  if(!check(lexer::TokenType::rbrace)) {
    do {
      expect(lexer::TokenType::identifier, "expected record field name");
      auto field = previous;
      expect(lexer::TokenType::equal_op,
             "expected '=' after record field name");
      auto exp = expression(Precedence::None);
      fields.emplace_back(Symbol(field.value), std::move(exp), field.pos);
    } while(match(lexer::TokenType::comma));
  }

  expect(lexer::TokenType::rbrace, "expected '}' after record fields");
  return std::make_unique<ast::RecordExp>(
    type_sym, std::move(fields), type_pos);
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
  throw std::runtime_error(
    std::format("[line {}] Err at {}: {}\n", tok.pos.line, tok, err_msg));
}

ast::Operator Parser::map_operator(lexer::TokenType type)
{
  switch(type) {
  case lexer::TokenType::plus_op:
    return ast::Operator::plus;
  case lexer::TokenType::minus_op:
    return ast::Operator::minus;
  case lexer::TokenType::times_op:
    return ast::Operator::times;
  case lexer::TokenType::divide_op:
    return ast::Operator::divide;
  case lexer::TokenType::equal_op:
    return ast::Operator::equal;
  case lexer::TokenType::not_equal_op:
    return ast::Operator::not_equal;
  case lexer::TokenType::less_op:
    return ast::Operator::less;
  case lexer::TokenType::less_equal_op:
    return ast::Operator::less_equal;
  case lexer::TokenType::greater_op:
    return ast::Operator::greater;
  case lexer::TokenType::greater_equal_op:
    return ast::Operator::greater_equal;
  }
  assert(false);
  std::unreachable();
}

bool Parser::is_comparison_operator(ast::Operator type)
{
  switch(type) {
  case ast::Operator::equal:
  case ast::Operator::not_equal:
  case ast::Operator::less:
  case ast::Operator::less_equal:
  case ast::Operator::greater:
  case ast::Operator::greater_equal:
    return true;
  }
  return false;
}

} // namespace parser