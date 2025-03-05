#include <semantic/type_checker.hpp>

namespace semantic
{

void TypeChecker::check(const parser::ast::Expression& exp)
{
  //TODO
}

TypeChecker::type_t
TypeChecker::visit_string_exp(const parser::ast::StringExp& exp)
{
  // TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_assign_exp(const parser::ast::AssignExp& exp)
{
  // TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_op_exp(const parser::ast::OpExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_int_exp(const parser::ast::IntExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_var_exp(const parser::ast::VarExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_seq_exp(const parser::ast::SeqExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_array_exp(const parser::ast::ArrayExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_nil_exp(const parser::ast::NilExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_record_exp(const parser::ast::RecordExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_if_exp(const parser::ast::IfExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_break_exp(const parser::ast::BreakExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_while_exp(const parser::ast::WhileExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_for_exp(const parser::ast::ForExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_call_exp(const parser::ast::CallExp& exp)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_let_exp(const parser::ast::LetExp& exp)
{
  //TODO
  return nullptr;
};

void TypeChecker::visit_func_decl(const parser::ast::FuncDecl& decl) {
  //TODO
};
void TypeChecker::visit_var_decl(const parser::ast::VarDecl& decl) {
  //TODO
};
void TypeChecker::visit_type_decl(const parser::ast::TypeDecl& decl) {
  //TODO
};

TypeChecker::type_t
TypeChecker::visit_named_type(const parser::ast::NameType& type)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_array_type(const parser::ast::ArrayType& type)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_record_type(const parser::ast::RecordType& type)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_simple_var(const parser::ast::SimpleVar& var)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_field_var(const parser::ast::FieldVar& var)
{
  //TODO
  return nullptr;
};

TypeChecker::type_t
TypeChecker::visit_subscript_var(const parser::ast::SubscriptVar& var)
{
  //TODO
  return nullptr;
};

} // namespace semantic