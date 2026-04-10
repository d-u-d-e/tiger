#pragma once
#include "semant/env.hpp"
#include "semant/visitor.hpp"

namespace semant
{
struct Escape
{
  Escape(int depth, std::shared_ptr<bool> ref)
    : depth(depth)
    , ref(std::move(ref))
  { }
  [[nodiscard]] auto to_string() const -> std::string
  {
    return std::format("depth: {}, escape: {}", depth, *ref);
  }
  int depth;
  std::shared_ptr<bool> ref;
};

class EscapeFinder : public FindEscapeExprVisitor,
                     public FindEscapeDeclVisitor,
                     public FindEscapeVarVisitor
{

  public:
  EscapeFinder() = default;
  ~EscapeFinder() override = default;
  EscapeFinder(const EscapeFinder&) = delete;
  auto operator=(const EscapeFinder&) -> EscapeFinder& = delete;
  EscapeFinder(EscapeFinder&&) = delete;
  auto operator=(EscapeFinder&&) -> EscapeFinder& = delete;

  void visit_string_exp(parser::ast::StringExp& exp) override;
  void visit_assign_exp(parser::ast::AssignExp& exp) override;
  void visit_op_exp(parser::ast::OpExp& exp) override;
  void visit_int_exp(parser::ast::IntExp& exp) override;
  void visit_var_exp(parser::ast::VarExp& exp) override;
  void visit_seq_exp(parser::ast::SeqExp& exp) override;
  void visit_array_exp(parser::ast::ArrayExp& exp) override;
  void visit_nil_exp(parser::ast::NilExp& exp) override;
  void visit_record_exp(parser::ast::RecordExp& exp) override;
  void visit_if_exp(parser::ast::IfExp& exp) override;
  void visit_break_exp(parser::ast::BreakExp& exp) override;
  void visit_while_exp(parser::ast::WhileExp& exp) override;
  void visit_for_exp(parser::ast::ForExp& exp) override;
  void visit_call_exp(parser::ast::CallExp& exp) override;
  void visit_let_exp(parser::ast::LetExp& exp) override;

  void visit_func_decl(parser::ast::FuncDecl& decl) override;
  void visit_var_decl(parser::ast::VarDecl& decl) override;
  void visit_type_decl(parser::ast::TypeDecl& decl) override;

  void visit_var(parser::ast::Var& var) override;
  void visit_field_var(parser::ast::FieldVar& var) override;
  void visit_subscript_var(parser::ast::SubscriptVar& var) override;

  auto lookup(const Symbol& name) -> Escape
  {
    const auto* v = env.lookup(name);
    assert(v != nullptr);
    return *v;
  }

  private:
  Environment<Escape> env;
};

} // namespace semant