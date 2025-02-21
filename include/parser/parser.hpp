#pragma once
#include <deque>
#include <functional>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>

namespace parser
{

enum Precedence
{
  None = 0,
  Assignment,
  Or,
  And,
  Comparison,
  Term,
  Factor,
  Unary,
  Call,
  Primary,
};

class PrecedenceRule {
  public:
  PrecedenceRule(int precedence_value,
                 std::function<std::shared_ptr<ast::Expression>()> prefix_rule,
                 std::function<std::shared_ptr<ast::Expression>(
                   std::shared_ptr<ast::Expression>)> infix_rule)
    : precedence_value(precedence_value)
    , prefix_rule(prefix_rule)
    , infix_rule(infix_rule)
  { }
  int precedence_value;
  std::function<std::shared_ptr<ast::Expression>()> prefix_rule;
  std::function<std::shared_ptr<ast::Expression>(std::shared_ptr<ast::Expression> lhs)> infix_rule;
};

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
  std::unordered_map<lexer::TokenType, PrecedenceRule> pratt_table;

  std::shared_ptr<ast::Expression> expression(int precedence);
  std::shared_ptr<ast::SeqExp> sequencing();
  std::shared_ptr<ast::VarExp> simple_var();
  std::shared_ptr<ast::VarExp> field_var(std::shared_ptr<ast::Expression> lhs);
  std::shared_ptr<ast::Expression> array_subscript(std::shared_ptr<ast::Expression> lhs);
  std::shared_ptr<ast::IntExp> integer_literal();
  std::shared_ptr<ast::StringExp> string_literal();
  std::shared_ptr<ast::WhileExp> while_expr();
  std::shared_ptr<ast::ForExp> for_expr();
  std::shared_ptr<ast::BreakExp> break_expr();
  std::shared_ptr<ast::LetExp> let_expr();
  std::shared_ptr<ast::IfExp> if_expr();
  std::shared_ptr<ast::NilExp> nil_literal();
  std::shared_ptr<ast::OpExp> binary_expr(std::shared_ptr<ast::Expression> lhs);
  std::shared_ptr<ast::OpExp> unary_expr();
  std::shared_ptr<ast::OpExp> and_expr(std::shared_ptr<ast::Expression> lhs);
  std::shared_ptr<ast::OpExp> or_expr(std::shared_ptr<ast::Expression> lhs);
  std::shared_ptr<ast::AssignExp> assign_expr(std::shared_ptr<ast::Expression> lhs);
  std::shared_ptr<ast::CallExp> call_expr(std::shared_ptr<ast::Expression> lhs);
  std::shared_ptr<ast::Expression> record_expr(std::shared_ptr<ast::Expression> lhs);
};

} // namespace parser