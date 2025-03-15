#include <seman/escape.hpp>

namespace seman
{

void EscapeFinder::visit_string_exp(const parser::ast::StringExp& exp) { }
void EscapeFinder::visit_assign_exp(const parser::ast::AssignExp& exp) { }
void EscapeFinder::visit_op_exp(const parser::ast::OpExp& exp) { }
void EscapeFinder::visit_int_exp(const parser::ast::IntExp& exp) { }
void EscapeFinder::visit_var_exp(const parser::ast::VarExp& exp) { }
void EscapeFinder::visit_seq_exp(const parser::ast::SeqExp& exp) { }
void EscapeFinder::visit_array_exp(const parser::ast::ArrayExp& exp) { }
void EscapeFinder::visit_nil_exp(const parser::ast::NilExp& exp) { }
void EscapeFinder::visit_record_exp(const parser::ast::RecordExp& exp) { }
void EscapeFinder::visit_if_exp(const parser::ast::IfExp& exp) { }
void EscapeFinder::visit_break_exp(const parser::ast::BreakExp& exp) { }
void EscapeFinder::visit_while_exp(const parser::ast::WhileExp& exp) { }
void EscapeFinder::visit_for_exp(const parser::ast::ForExp& exp) { }
void EscapeFinder::visit_call_exp(const parser::ast::CallExp& exp) { }
void EscapeFinder::visit_let_exp(const parser::ast::LetExp& exp) { }

void EscapeFinder::visit_func_decl(const parser::ast::FuncDecl& decl) { }
void EscapeFinder::visit_var_decl(const parser::ast::VarDecl& decl) { }
void EscapeFinder::visit_type_decl(const parser::ast::TypeDecl& decl) { }

void EscapeFinder::visit_name_type(const parser::ast::NameType& type) { }
void EscapeFinder::visit_array_type(const parser::ast::ArrayType& type) { }
void EscapeFinder::visit_record_type(const parser::ast::RecordType& type) { }

void EscapeFinder::visit_simple_var(const parser::ast::SimpleVar& var) { }
void EscapeFinder::visit_field_var(const parser::ast::FieldVar& var) { }
void EscapeFinder::visit_subscript_var(const parser::ast::SubscriptVar& var) { }

} // namespace seman