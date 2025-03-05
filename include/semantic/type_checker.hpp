#pragma once
#include <parser/ast.hpp>
#include <parser/visitor.hpp>
#include <semantic/env.hpp>
#include <symbol.hpp>

namespace semantic
{

using namespace env;

class TypeChecker : public parser::ast::TypeCheckerExprVisitor,
                    public parser::ast::TypeCheckerDeclVisitor,
                    public parser::ast::TypeCheckerVarVisitor,
                    public parser::ast::TypeCheckerTypeVisitor

{

  public:
  TypeChecker(symbol::StringTable& string_table);
  void check(const parser::ast::Expression& exp);

  TEntry visit_string_exp(const parser::ast::StringExp& exp) override;
  TEntry visit_assign_exp(const parser::ast::AssignExp& exp) override;
  TEntry visit_op_exp(const parser::ast::OpExp& exp) override;
  TEntry visit_int_exp(const parser::ast::IntExp& exp) override;
  TEntry visit_var_exp(const parser::ast::VarExp& exp) override;
  TEntry visit_seq_exp(const parser::ast::SeqExp& exp) override;
  TEntry visit_array_exp(const parser::ast::ArrayExp& exp) override;
  TEntry visit_nil_exp(const parser::ast::NilExp& exp) override;
  TEntry visit_record_exp(const parser::ast::RecordExp& exp) override;
  TEntry visit_if_exp(const parser::ast::IfExp& exp) override;
  TEntry visit_break_exp(const parser::ast::BreakExp& exp) override;
  TEntry visit_while_exp(const parser::ast::WhileExp& exp) override;
  TEntry visit_for_exp(const parser::ast::ForExp& exp) override;
  TEntry visit_call_exp(const parser::ast::CallExp& exp) override;
  TEntry visit_let_exp(const parser::ast::LetExp& exp) override;

  void visit_func_decl(const parser::ast::FuncDecl& decl) override;
  void visit_var_decl(const parser::ast::VarDecl& decl) override;
  void visit_type_decl(const parser::ast::TypeDecl& decl) override;

  TEntry visit_name_type(const parser::ast::NameType& type) override;
  TEntry visit_array_type(const parser::ast::ArrayType& type) override;
  TEntry visit_record_type(const parser::ast::RecordType& type) override;

  TEntry visit_simple_var(const parser::ast::SimpleVar& var) override;
  TEntry visit_field_var(const parser::ast::FieldVar& var) override;
  TEntry visit_subscript_var(const parser::ast::SubscriptVar& var) override;

  private:
  template <typename T>
  bool check_type(const types::Type& t);
  bool is_same_type(const TEntry& t1, const TEntry& t2)
  {
    return t1 == t2;
  }
  void error_at(const lexer::Position& pos, const std::string& err_msg);

  symbol::StringTable& string_table;
  Environment<TEntry> tenv;
  Environment<VEntry> venv;
};
} // namespace semantic