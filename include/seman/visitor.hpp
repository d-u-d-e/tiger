#pragma once
#include <seman/types.hpp>

namespace parser::ast
{
class SimpleVar;
class FieldVar;
class StringExp;
class AssignExp;
class OpExp;
class IntExp;
class VarExp;
class SeqExp;
class SubscriptVar;
class ArrayExp;
class NilExp;
class RecordExp;
class IfExp;
class BreakExp;
class WhileExp;
class ForExp;
class CallExp;
class LetExp;
class FuncDecl;
class VarDecl;
class TypeDecl;
class NameType;
class ArrayType;
class RecordType;
} // namespace parser::ast

namespace seman
{

class TypeCheckerExprVisitor {
  public:
  // clang-format off
  types::shared_type_t virtual visit_string_exp(const parser::ast::StringExp& exp) = 0;
  types::shared_type_t virtual visit_assign_exp(const parser::ast::AssignExp& exp) = 0;
  types::shared_type_t virtual visit_op_exp(const parser::ast::OpExp& exp) = 0;
  types::shared_type_t virtual visit_int_exp(const parser::ast::IntExp& exp) = 0;
  types::shared_type_t virtual visit_var_exp(const parser::ast::VarExp& exp) = 0;
  types::shared_type_t virtual visit_seq_exp(const parser::ast::SeqExp& exp) = 0;
  types::shared_type_t virtual visit_array_exp(const parser::ast::ArrayExp& exp) = 0;
  types::shared_type_t virtual visit_nil_exp(const parser::ast::NilExp& exp) = 0;
  types::shared_type_t virtual visit_record_exp(const parser::ast::RecordExp& exp) = 0;
  types::shared_type_t virtual visit_if_exp(const parser::ast::IfExp& exp) = 0;
  types::shared_type_t virtual visit_break_exp(const parser::ast::BreakExp& exp) = 0;
  types::shared_type_t virtual visit_while_exp(const parser::ast::WhileExp& exp) = 0;
  types::shared_type_t virtual visit_for_exp(const parser::ast::ForExp& exp) = 0;
  types::shared_type_t virtual visit_call_exp(const parser::ast::CallExp& exp) = 0;
  types::shared_type_t virtual visit_let_exp(const parser::ast::LetExp& exp) = 0;
  // clang-format on
};

class TypeCheckerDeclVisitor {
  public:
  // clang-format off
  void virtual visit_func_decl(const parser::ast::FuncDecl& decl) = 0;
  void virtual visit_var_decl(const parser::ast::VarDecl& decl) = 0;
  void virtual visit_type_decl(const parser::ast::TypeDecl& decl) = 0;
  // clang-format on
};

class TypeCheckerTypeVisitor {
  public:
  // clang-format off
  types::shared_type_t virtual visit_name_type(const parser::ast::NameType& type) = 0;
  types::shared_type_t virtual visit_array_type(const parser::ast::ArrayType& type) = 0;
  types::shared_type_t virtual visit_record_type(const parser::ast::RecordType& type) = 0;
  // clang-format on
};

class TypeCheckerVarVisitor {
  public:
  // clang-format off
  types::shared_type_t virtual visit_simple_var(const parser::ast::SimpleVar& var) = 0;
  types::shared_type_t virtual visit_field_var(const parser::ast::FieldVar& var) = 0;
  types::shared_type_t virtual visit_subscript_var(const parser::ast::SubscriptVar& var) = 0;
  // clang-format on
};
}; // namespace seman
