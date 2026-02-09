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
  types::Result virtual visit_string_exp(const parser::ast::StringExp& exp) = 0;
  types::Result virtual visit_assign_exp(const parser::ast::AssignExp& exp) = 0;
  types::Result virtual visit_op_exp(const parser::ast::OpExp& exp) = 0;
  types::Result virtual visit_int_exp(const parser::ast::IntExp& exp) = 0;
  types::Result virtual visit_var_exp(const parser::ast::VarExp& exp) = 0;
  types::Result virtual visit_seq_exp(const parser::ast::SeqExp& exp) = 0;
  types::Result virtual visit_array_exp(const parser::ast::ArrayExp& exp) = 0;
  types::Result virtual visit_nil_exp(const parser::ast::NilExp& exp) = 0;
  types::Result virtual visit_record_exp(const parser::ast::RecordExp& exp) = 0;
  types::Result virtual visit_if_exp(const parser::ast::IfExp& exp) = 0;
  types::Result virtual visit_break_exp(const parser::ast::BreakExp& exp) = 0;
  types::Result virtual visit_while_exp(const parser::ast::WhileExp& exp) = 0;
  types::Result virtual visit_for_exp(const parser::ast::ForExp& exp) = 0;
  types::Result virtual visit_call_exp(const parser::ast::CallExp& exp) = 0;
  types::Result virtual visit_let_exp(const parser::ast::LetExp& exp) = 0;
};

class TypeCheckerDeclVisitor
{
  public:
  types::Result virtual visit_func_decl(const parser::ast::FuncDecl& decl) = 0;
  types::Result virtual visit_var_decl(const parser::ast::VarDecl& decl) = 0;
  types::Result virtual visit_type_decl(const parser::ast::TypeDecl& decl) = 0;
};

class TypeCheckerTypeVisitor
{
  public:
  types::SharedType virtual visit_name_type(const parser::ast::NameType& type) = 0;
  types::SharedType virtual visit_array_type(const parser::ast::ArrayType& type) = 0;
  types::SharedType virtual visit_record_type(const parser::ast::RecordType& type) = 0;
  types::SharedType virtual visit_func_type(const parser::ast::FunctionType& type) = 0;
};

class TypeCheckerVarVisitor
{
  public:
  types::Result virtual visit_var(const parser::ast::Var& var) = 0;
  types::Result virtual visit_field_var(const parser::ast::FieldVar& var) = 0;
  types::Result virtual visit_subscript_var(const parser::ast::SubscriptVar& var) = 0;
};

class FindEscapeExprVisitor
{
  public:
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
  void virtual visit_func_decl(parser::ast::FuncDecl& decl) = 0;
  void virtual visit_var_decl(parser::ast::VarDecl& decl) = 0;
  void virtual visit_type_decl(parser::ast::TypeDecl& decl) = 0;
};

class FindEscapeVarVisitor
{
  public:
  void virtual visit_var(parser::ast::Var& var) = 0;
  void virtual visit_field_var(parser::ast::FieldVar& var) = 0;
  void virtual visit_subscript_var(parser::ast::SubscriptVar& var) = 0;
};

}; // namespace semant
