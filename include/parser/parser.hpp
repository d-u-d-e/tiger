#pragma once
#include <functional>
#include <lexer/lex.hpp>
#include <lexer/token.hpp>
#include <memory>
#include <ostream>
#include <parser/ast.hpp>
#include <string>
#include <symbol.hpp>
#include <unordered_map>
#include <unordered_set>
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

class PrecedenceRule {
  public:
  PrecedenceRule(int precedence_value,
                 std::function<std::unique_ptr<ast::Expression>()> prefix_rule,
                 std::function<std::unique_ptr<ast::Expression>(
                   std::unique_ptr<ast::Expression>)> infix_rule)
    : precedence_value(precedence_value)
    , prefix_rule(prefix_rule)
    , infix_rule(infix_rule)
  { }
  int precedence_value;
  std::function<std::unique_ptr<ast::Expression>()> prefix_rule;
  std::function<std::unique_ptr<ast::Expression>(
    std::unique_ptr<ast::Expression> lhs)>
    infix_rule;
};

class Parser {
  public:
  Parser(std::ostream& ostream,
         lexer::Scanner& scanner,
         symbol::StringTable& symbol_table)
    : ostream(ostream)
    , scanner(scanner)
    , symbol_table(symbol_table){};
  std::unique_ptr<ast::Expression> parse();

  bool had_error()
  {
    return had_error_;
  }

  private:
  std::ostream& ostream;
  lexer::Scanner& scanner;
  symbol::StringTable& symbol_table;
  lexer::Token current;
  lexer::Token previous;
  std::unordered_map<lexer::TokenType, PrecedenceRule> pratt_table;

  inline const symbol::Symbol& symbol(const std::string& name)
  {
    return symbol_table.symbol(name);
  }

  void skip(const std::unordered_set<lexer::TokenType>& list);

  ast::Operator map_operator(lexer::TokenType type);
  bool is_comparison_operator(ast::Operator type);

  bool match(lexer::TokenType type);
  void expect(lexer::TokenType type, const std::string& err_msg);
  void advance();
  bool check(lexer::TokenType type);
  void error_at(const lexer::Token& tok, const std::string& err_msg);

  std::unique_ptr<ast::Expression> expression(int precedence);
  std::unique_ptr<ast::SeqExp> sequencing();
  std::unique_ptr<ast::VarExp> variable();
  std::unique_ptr<ast::VarExp>
  record_field(std::unique_ptr<ast::Expression> lhs);
  std::unique_ptr<ast::Expression>
  array_subscript(std::unique_ptr<ast::Expression> lhs);
  std::unique_ptr<ast::IntExp> integer_literal();
  std::unique_ptr<ast::StringExp> string_literal();
  std::unique_ptr<ast::WhileExp> while_expr();
  std::unique_ptr<ast::ForExp> for_expr();
  std::unique_ptr<ast::BreakExp> break_expr();
  std::unique_ptr<ast::LetExp> let_expr();
  std::unique_ptr<ast::IfExp> if_expr();
  std::unique_ptr<ast::NilExp> nil_literal();
  std::unique_ptr<ast::OpExp> binary_expr(std::unique_ptr<ast::Expression> lhs);
  std::unique_ptr<ast::OpExp> unary_expr();
  std::unique_ptr<ast::Expression>
  and_expr(std::unique_ptr<ast::Expression> lhs);
  std::unique_ptr<ast::Expression>
  or_expr(std::unique_ptr<ast::Expression> lhs);
  std::unique_ptr<ast::AssignExp>
  assign_expr(std::unique_ptr<ast::Expression> lhs);
  std::unique_ptr<ast::CallExp> call_expr(std::unique_ptr<ast::Expression> lhs);
  std::unique_ptr<ast::Expression>
  record_expr(std::unique_ptr<ast::Expression> lhs);

  std::unique_ptr<ast::Declaration> decl();
  std::vector<std::unique_ptr<ast::Declaration>> decls();
  std::unique_ptr<ast::FuncDecl> func_decl();
  std::unique_ptr<ast::TypeDecl> type_decl();
  std::unique_ptr<ast::VarDecl> var_decl();

  bool had_error_{false};
};

} // namespace parser