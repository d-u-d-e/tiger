#pragma once
#include "lexer/lex.hpp"
#include "lexer/token.hpp"
#include "parser/ast.hpp"
#include "string_table.hpp"
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <symbol.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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

class PrecedenceRule
{
  public:
  PrecedenceRule(
    int precedence_value,
    std::function<std::unique_ptr<ast::Expression>()> prefix_rule,
    std::function<std::unique_ptr<ast::Expression>(std::unique_ptr<ast::Expression>)> infix_rule)
    : precedence_value(precedence_value)
    , prefix_rule(std::move(prefix_rule))
    , infix_rule(std::move(infix_rule))
  { }
  int precedence_value;
  std::function<std::unique_ptr<ast::Expression>()> prefix_rule;
  std::function<std::unique_ptr<ast::Expression>(std::unique_ptr<ast::Expression> lhs)> infix_rule;
};

class Parser
{
  public:
  class Exception : public std::runtime_error
  {
public:
    explicit Exception(const std::string& what)
      : std::runtime_error(what)
    { }
  };

  Parser(std::ostream& ostream, lexer::Scanner& scanner, StringTable& symbol_table)
    : ostream(ostream)
    , scanner(scanner)
    , symbol_table(symbol_table){};
  auto parse() -> std::unique_ptr<ast::Expression>;

  auto had_error() const -> bool
  {
    return had_error_;
  }

  Parser(Parser const&) = delete;
  auto operator=(Parser const&) -> Parser& = delete;
  Parser(Parser&&) = delete;
  auto operator=(Parser&&) -> Parser& = delete;
  ~Parser() = default;

  private:
  std::ostream& ostream;
  lexer::Scanner& scanner;
  StringTable& symbol_table;
  lexer::Token current;
  lexer::Token previous;
  std::unordered_map<lexer::TokenType, PrecedenceRule> pratt_table;

  auto symbol(const std::string& name) -> const Symbol&
  {
    return symbol_table.symbol(name);
  }

  void skip(const std::unordered_set<lexer::TokenType>& list);

  auto static map_operator(lexer::TokenType type) -> ast::Operator;
  auto static is_comparison_operator(ast::Operator type) -> bool;

  auto match(lexer::TokenType type) -> bool;
  void expect(lexer::TokenType type, const std::string& err_msg);
  void advance();
  auto check(lexer::TokenType type) const -> bool;
  void error_at(const lexer::Token& tok, const std::string& err_msg);

  auto expression(int precedence) -> std::unique_ptr<ast::Expression>;
  auto sequencing() -> std::unique_ptr<ast::SeqExp>;
  auto variable() -> std::unique_ptr<ast::VarExp>;
  auto record_field(std::unique_ptr<ast::Expression> lhs) -> std::unique_ptr<ast::VarExp>;
  auto array_subscript(std::unique_ptr<ast::Expression> lhs) -> std::unique_ptr<ast::Expression>;
  auto integer_literal() -> std::unique_ptr<ast::IntExp>;
  auto string_literal() -> std::unique_ptr<ast::StringExp>;
  auto while_expr() -> std::unique_ptr<ast::WhileExp>;
  auto for_expr() -> std::unique_ptr<ast::ForExp>;
  auto break_expr() -> std::unique_ptr<ast::BreakExp>;
  auto let_expr() -> std::unique_ptr<ast::LetExp>;
  auto if_expr() -> std::unique_ptr<ast::IfExp>;
  auto static nil_literal() -> std::unique_ptr<ast::NilExp>;
  auto binary_expr(std::unique_ptr<ast::Expression> lhs) -> std::unique_ptr<ast::OpExp>;
  auto unary_expr() -> std::unique_ptr<ast::Expression>;
  auto and_expr(std::unique_ptr<ast::Expression> lhs) -> std::unique_ptr<ast::Expression>;
  auto or_expr(std::unique_ptr<ast::Expression> lhs) -> std::unique_ptr<ast::Expression>;
  auto assign_expr(std::unique_ptr<ast::Expression> lhs) -> std::unique_ptr<ast::AssignExp>;
  auto call_expr(std::unique_ptr<ast::Expression> lhs) -> std::unique_ptr<ast::CallExp>;
  auto record_expr(std::unique_ptr<ast::Expression> lhs) -> std::unique_ptr<ast::Expression>;

  auto decl() -> std::unique_ptr<ast::Declaration>;
  auto decls() -> std::vector<std::unique_ptr<ast::Declaration>>;
  auto func_decl() -> std::unique_ptr<ast::FuncDecl>;
  auto type_decl() -> std::unique_ptr<ast::TypeDecl>;
  auto var_decl() -> std::unique_ptr<ast::VarDecl>;

  auto ty() -> std::unique_ptr<ast::Type>;

  bool had_error_{false};
};

} // namespace parser