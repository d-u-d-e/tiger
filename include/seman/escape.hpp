#pragma once
#include <seman/visitor.hpp>
#include <symbol.hpp>

namespace seman
{

class EscapeFinder : public FindEscapeExprVisitor,
                     public FindEscapeDeclVisitor,
                     public FindEscapeTypeVisitor,
                     public FindEscapeVarVisitor {

  public:
  struct Escape {
    int depth;
    bool value;
  };

  EscapeFinder(symbol::Table<Escape>& table)
    : table(table)
  { }

  void visit_string_exp(const parser::ast::StringExp& exp) override;
  void visit_assign_exp(const parser::ast::AssignExp& exp) override;
  void visit_op_exp(const parser::ast::OpExp& exp) override;
  void visit_int_exp(const parser::ast::IntExp& exp) override;
  void visit_var_exp(const parser::ast::VarExp& exp) override;
  void visit_seq_exp(const parser::ast::SeqExp& exp) override;
  void visit_array_exp(const parser::ast::ArrayExp& exp) override;
  void visit_nil_exp(const parser::ast::NilExp& exp) override;
  void visit_record_exp(const parser::ast::RecordExp& exp) override;
  void visit_if_exp(const parser::ast::IfExp& exp) override;
  void visit_break_exp(const parser::ast::BreakExp& exp) override;
  void visit_while_exp(const parser::ast::WhileExp& exp) override;
  void visit_for_exp(const parser::ast::ForExp& exp) override;
  void visit_call_exp(const parser::ast::CallExp& exp) override;
  void visit_let_exp(const parser::ast::LetExp& exp) override;

  void visit_func_decl(const parser::ast::FuncDecl& decl) override;
  void visit_var_decl(const parser::ast::VarDecl& decl) override;
  void visit_type_decl(const parser::ast::TypeDecl& decl) override;

  void visit_name_type(const parser::ast::NameType& type) override;
  void visit_array_type(const parser::ast::ArrayType& type) override;
  void visit_record_type(const parser::ast::RecordType& type) override;

  void visit_simple_var(const parser::ast::SimpleVar& var) override;
  void visit_field_var(const parser::ast::FieldVar& var) override;
  void visit_subscript_var(const parser::ast::SubscriptVar& var) override;

  private:
  symbol::Table<Escape>& table;
};

} // namespace seman