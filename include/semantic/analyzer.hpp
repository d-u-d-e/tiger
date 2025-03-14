#pragma once
#include <parser/ast.hpp>
#include <parser/visitor.hpp>
#include <semantic/env.hpp>
#include <symbol.hpp>

namespace semantic
{

using namespace env;

class Analyzer : public parser::ast::TypeCheckerExprVisitor,
                 public parser::ast::TypeCheckerDeclVisitor,
                 public parser::ast::TypeCheckerVarVisitor,
                 public parser::ast::TypeCheckerTypeVisitor

{

  public:
  Analyzer(symbol::StringTable& string_table);
  void type_check(const parser::ast::Expression& exp);

  shared_type_t visit_string_exp(const parser::ast::StringExp& exp) override;
  shared_type_t visit_assign_exp(const parser::ast::AssignExp& exp) override;
  shared_type_t visit_op_exp(const parser::ast::OpExp& exp) override;
  shared_type_t visit_int_exp(const parser::ast::IntExp& exp) override;
  shared_type_t visit_var_exp(const parser::ast::VarExp& exp) override;
  shared_type_t visit_seq_exp(const parser::ast::SeqExp& exp) override;
  shared_type_t visit_array_exp(const parser::ast::ArrayExp& exp) override;
  shared_type_t visit_nil_exp(const parser::ast::NilExp& exp) override;
  shared_type_t visit_record_exp(const parser::ast::RecordExp& exp) override;
  shared_type_t visit_if_exp(const parser::ast::IfExp& exp) override;
  shared_type_t visit_break_exp(const parser::ast::BreakExp& exp) override;
  shared_type_t visit_while_exp(const parser::ast::WhileExp& exp) override;
  shared_type_t visit_for_exp(const parser::ast::ForExp& exp) override;
  shared_type_t visit_call_exp(const parser::ast::CallExp& exp) override;
  shared_type_t visit_let_exp(const parser::ast::LetExp& exp) override;

  void visit_func_decl(const parser::ast::FuncDecl& decl) override;
  void visit_var_decl(const parser::ast::VarDecl& decl) override;
  void visit_type_decl(const parser::ast::TypeDecl& decl) override;

  shared_type_t visit_name_type(const parser::ast::NameType& type) override;
  shared_type_t visit_array_type(const parser::ast::ArrayType& type) override;
  shared_type_t visit_record_type(const parser::ast::RecordType& type) override;

  shared_type_t visit_simple_var(const parser::ast::SimpleVar& var) override;
  shared_type_t visit_field_var(const parser::ast::FieldVar& var) override;
  shared_type_t
  visit_subscript_var(const parser::ast::SubscriptVar& var) override;

  private:
  void add_predefined_types();
  void add_predefined_functions();
  template <typename... Args>
  void add_predef_func(const symbol::Symbol& s,
                       const shared_type_t& ret,
                       Args&&... formals);

  template <typename T>
  bool is_type(const shared_type_t& t);
  bool can_assign(const shared_type_t& tlhs, const shared_type_t& trhs);
  bool same_types(const shared_type_t& t1, const shared_type_t& t2)
  {
    return t1 == t2;
  }
  void error_at(const lexer::Position& pos, const std::string& err_msg);

  void detect_cycles(const parser::ast::TypeDecl& decl);
  shared_type_t skip_name_types(const shared_type_t& t);
  bool can_break{false};
  symbol::StringTable& string_table;
  Environment<TEntry> tenv;
  Environment<VEntry> venv;
};
} // namespace semantic