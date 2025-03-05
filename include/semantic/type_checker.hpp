#pragma once
#include <parser/ast.hpp>
#include <parser/visitor.hpp>
#include <semantic/env.hpp>

namespace semantic
{

using namespace env;

class TypeChecker
  : public parser::ast::ExprVisitor<std::shared_ptr<semantic::types::Type>>,
    public parser::ast::DeclVisitor<void>,
    public parser::ast::TypeVisitor<std::shared_ptr<semantic::types::Type>>,
    public parser::ast::VarVisitor<std::shared_ptr<semantic::types::Type>>

{

  public:
  using type_t = std::shared_ptr<semantic::types::Type>;

  TypeChecker(std::shared_ptr<Environment<TEntry>> tenv,
              std::shared_ptr<Environment<VEntry>> venv)
    : tenv(std::move(tenv))
    , venv(std::move(venv))
  { }

  void check(const parser::ast::Expression& exp);

  type_t visit_string_exp(const parser::ast::StringExp& exp) override;
  type_t visit_assign_exp(const parser::ast::AssignExp& exp) override;
  type_t visit_op_exp(const parser::ast::OpExp& exp) override;
  type_t visit_int_exp(const parser::ast::IntExp& exp) override;
  type_t visit_var_exp(const parser::ast::VarExp& exp) override;
  type_t visit_seq_exp(const parser::ast::SeqExp& exp) override;
  type_t visit_array_exp(const parser::ast::ArrayExp& exp) override;
  type_t visit_nil_exp(const parser::ast::NilExp& exp) override;
  type_t visit_record_exp(const parser::ast::RecordExp& exp) override;
  type_t visit_if_exp(const parser::ast::IfExp& exp) override;
  type_t visit_break_exp(const parser::ast::BreakExp& exp) override;
  type_t visit_while_exp(const parser::ast::WhileExp& exp) override;
  type_t visit_for_exp(const parser::ast::ForExp& exp) override;
  type_t visit_call_exp(const parser::ast::CallExp& exp) override;
  type_t visit_let_exp(const parser::ast::LetExp& exp) override;

  void visit_func_decl(const parser::ast::FuncDecl& decl) override;
  void visit_var_decl(const parser::ast::VarDecl& decl) override;
  void visit_type_decl(const parser::ast::TypeDecl& decl) override;

  type_t visit_named_type(const parser::ast::NameType& type) override;
  type_t visit_array_type(const parser::ast::ArrayType& type) override;
  type_t visit_record_type(const parser::ast::RecordType& type) override;

  type_t visit_simple_var(const parser::ast::SimpleVar& var) override;
  type_t visit_field_var(const parser::ast::FieldVar& var) override;
  type_t
  visit_subscript_var(const parser::ast::SubscriptVar& var) override;

  private:
  std::shared_ptr<Environment<TEntry>> tenv;
  std::shared_ptr<Environment<VEntry>> venv;
};
} // namespace semantic