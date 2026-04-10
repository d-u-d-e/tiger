#pragma once
#include <semant/types.hpp>

namespace parser::ast
{
class Var;
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
class FunctionType;
} // namespace parser::ast

namespace semant
{

class TypeCheckerExprVisitor
{
  public:
  virtual ~TypeCheckerExprVisitor() = default;
  TypeCheckerExprVisitor() = default;
  TypeCheckerExprVisitor(const TypeCheckerExprVisitor&) = delete;
  auto operator=(const TypeCheckerExprVisitor&) -> TypeCheckerExprVisitor& = delete;
  TypeCheckerExprVisitor(TypeCheckerExprVisitor&&) = delete;
  auto operator=(TypeCheckerExprVisitor&&) -> TypeCheckerExprVisitor& = delete;

  auto virtual visit_string_exp(const parser::ast::StringExp& exp) -> types::Result = 0;
  auto virtual visit_assign_exp(const parser::ast::AssignExp& exp) -> types::Result = 0;
  auto virtual visit_op_exp(const parser::ast::OpExp& exp) -> types::Result = 0;
  auto virtual visit_int_exp(const parser::ast::IntExp& exp) -> types::Result = 0;
  auto virtual visit_var_exp(const parser::ast::VarExp& exp) -> types::Result = 0;
  auto virtual visit_seq_exp(const parser::ast::SeqExp& exp) -> types::Result = 0;
  auto virtual visit_array_exp(const parser::ast::ArrayExp& exp) -> types::Result = 0;
  auto virtual visit_nil_exp(const parser::ast::NilExp& exp) -> types::Result = 0;
  auto virtual visit_record_exp(const parser::ast::RecordExp& exp) -> types::Result = 0;
  auto virtual visit_if_exp(const parser::ast::IfExp& exp) -> types::Result = 0;
  auto virtual visit_break_exp(const parser::ast::BreakExp& exp) -> types::Result = 0;
  auto virtual visit_while_exp(const parser::ast::WhileExp& exp) -> types::Result = 0;
  auto virtual visit_for_exp(const parser::ast::ForExp& exp) -> types::Result = 0;
  auto virtual visit_call_exp(const parser::ast::CallExp& exp) -> types::Result = 0;
  auto virtual visit_let_exp(const parser::ast::LetExp& exp) -> types::Result = 0;
};

class TypeCheckerDeclVisitor
{
  public:
  virtual ~TypeCheckerDeclVisitor() = default;
  TypeCheckerDeclVisitor() = default;
  TypeCheckerDeclVisitor(const TypeCheckerDeclVisitor&) = delete;
  auto operator=(const TypeCheckerDeclVisitor&) -> TypeCheckerDeclVisitor& = delete;
  TypeCheckerDeclVisitor(TypeCheckerDeclVisitor&&) = delete;
  auto operator=(TypeCheckerDeclVisitor&&) -> TypeCheckerDeclVisitor& = delete;

  auto virtual visit_func_decl(const parser::ast::FuncDecl& decl) -> types::Result = 0;
  auto virtual visit_var_decl(const parser::ast::VarDecl& decl) -> types::Result = 0;
  auto virtual visit_type_decl(const parser::ast::TypeDecl& decl) -> types::Result = 0;
};

class TypeCheckerTypeVisitor
{
  public:
  virtual ~TypeCheckerTypeVisitor() = default;
  TypeCheckerTypeVisitor() = default;
  TypeCheckerTypeVisitor(const TypeCheckerTypeVisitor&) = delete;
  auto operator=(const TypeCheckerTypeVisitor&) -> TypeCheckerTypeVisitor& = delete;
  TypeCheckerTypeVisitor(TypeCheckerTypeVisitor&&) = delete;
  auto operator=(TypeCheckerTypeVisitor&&) -> TypeCheckerTypeVisitor& = delete;

  auto virtual visit_name_type(const parser::ast::NameType& type) -> types::SharedType = 0;
  auto virtual visit_array_type(const parser::ast::ArrayType& type) -> types::SharedType = 0;
  auto virtual visit_record_type(const parser::ast::RecordType& type) -> types::SharedType = 0;
  auto virtual visit_func_type(const parser::ast::FunctionType& type) -> types::SharedType = 0;
};

class TypeCheckerVarVisitor
{
  public:
  virtual ~TypeCheckerVarVisitor() = default;
  TypeCheckerVarVisitor() = default;
  TypeCheckerVarVisitor(const TypeCheckerVarVisitor&) = delete;
  auto operator=(const TypeCheckerVarVisitor&) -> TypeCheckerVarVisitor& = delete;
  TypeCheckerVarVisitor(TypeCheckerVarVisitor&&) = delete;
  auto operator=(TypeCheckerVarVisitor&&) -> TypeCheckerVarVisitor& = delete;

  auto virtual visit_var(const parser::ast::Var& var) -> types::Result = 0;
  auto virtual visit_field_var(const parser::ast::FieldVar& var) -> types::Result = 0;
  auto virtual visit_subscript_var(const parser::ast::SubscriptVar& var) -> types::Result = 0;
};

class FindEscapeExprVisitor
{
  public:
  virtual ~FindEscapeExprVisitor() = default;
  FindEscapeExprVisitor() = default;
  FindEscapeExprVisitor(const FindEscapeExprVisitor&) = delete;
  auto operator=(const FindEscapeExprVisitor&) -> FindEscapeExprVisitor& = delete;
  FindEscapeExprVisitor(FindEscapeExprVisitor&&) = delete;
  auto operator=(FindEscapeExprVisitor&&) -> FindEscapeExprVisitor& = delete;

  void virtual visit_string_exp(parser::ast::StringExp& exp) = 0;
  void virtual visit_assign_exp(parser::ast::AssignExp& exp) = 0;
  void virtual visit_op_exp(parser::ast::OpExp& exp) = 0;
  void virtual visit_int_exp(parser::ast::IntExp& exp) = 0;
  void virtual visit_var_exp(parser::ast::VarExp& exp) = 0;
  void virtual visit_seq_exp(parser::ast::SeqExp& exp) = 0;
  void virtual visit_array_exp(parser::ast::ArrayExp& exp) = 0;
  void virtual visit_nil_exp(parser::ast::NilExp& exp) = 0;
  void virtual visit_record_exp(parser::ast::RecordExp& exp) = 0;
  void virtual visit_if_exp(parser::ast::IfExp& exp) = 0;
  void virtual visit_break_exp(parser::ast::BreakExp& exp) = 0;
  void virtual visit_while_exp(parser::ast::WhileExp& exp) = 0;
  void virtual visit_for_exp(parser::ast::ForExp& exp) = 0;
  void virtual visit_call_exp(parser::ast::CallExp& exp) = 0;
  void virtual visit_let_exp(parser::ast::LetExp& exp) = 0;
};

class FindEscapeDeclVisitor
{
  public:
  virtual ~FindEscapeDeclVisitor() = default;
  FindEscapeDeclVisitor() = default;
  FindEscapeDeclVisitor(const FindEscapeDeclVisitor&) = delete;
  auto operator=(const FindEscapeDeclVisitor&) -> FindEscapeDeclVisitor& = delete;
  FindEscapeDeclVisitor(FindEscapeDeclVisitor&&) = delete;
  auto operator=(FindEscapeDeclVisitor&&) -> FindEscapeDeclVisitor& = delete;

  void virtual visit_func_decl(parser::ast::FuncDecl& decl) = 0;
  void virtual visit_var_decl(parser::ast::VarDecl& decl) = 0;
  void virtual visit_type_decl(parser::ast::TypeDecl& decl) = 0;
};

class FindEscapeVarVisitor
{
  public:
  virtual ~FindEscapeVarVisitor() = default;
  FindEscapeVarVisitor() = default;
  FindEscapeVarVisitor(const FindEscapeVarVisitor&) = delete;
  auto operator=(const FindEscapeVarVisitor&) -> FindEscapeVarVisitor& = delete;
  FindEscapeVarVisitor(FindEscapeVarVisitor&&) = delete;
  auto operator=(FindEscapeVarVisitor&&) -> FindEscapeVarVisitor& = delete;

  void virtual visit_var(parser::ast::Var& var) = 0;
  void virtual visit_field_var(parser::ast::FieldVar& var) = 0;
  void virtual visit_subscript_var(parser::ast::SubscriptVar& var) = 0;
};

}; // namespace semant
