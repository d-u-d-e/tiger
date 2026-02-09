#pragma once
#include "parser/ast.hpp"
#include <string>

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

class PrettyPrinterExprVisitor
{
  public:
  std::string virtual visit_string_exp(const parser::ast::StringExp& exp) = 0;
  std::string virtual visit_assign_exp(const parser::ast::AssignExp& exp) = 0;
  std::string virtual visit_op_exp(const parser::ast::OpExp& exp) = 0;
  std::string virtual visit_int_exp(const parser::ast::IntExp& exp) = 0;
  std::string virtual visit_var_exp(const parser::ast::VarExp& exp) = 0;
  std::string virtual visit_seq_exp(const parser::ast::SeqExp& exp) = 0;
  std::string virtual visit_array_exp(const parser::ast::ArrayExp& exp) = 0;
  std::string virtual visit_nil_exp(const parser::ast::NilExp& exp) = 0;
  std::string virtual visit_record_exp(const parser::ast::RecordExp& exp) = 0;
  std::string virtual visit_if_exp(const parser::ast::IfExp& exp) = 0;
  std::string virtual visit_break_exp(const parser::ast::BreakExp& exp) = 0;
  std::string virtual visit_while_exp(const parser::ast::WhileExp& exp) = 0;
  std::string virtual visit_for_exp(const parser::ast::ForExp& exp) = 0;
  std::string virtual visit_call_exp(const parser::ast::CallExp& exp) = 0;
  std::string virtual visit_let_exp(const parser::ast::LetExp& exp) = 0;
};

class PrettyPrinterDeclVisitor
{
  public:
  std::string virtual visit_func_decl(const parser::ast::FuncDecl& decl) = 0;
  std::string virtual visit_var_decl(const parser::ast::VarDecl& decl) = 0;
  std::string virtual visit_type_decl(const parser::ast::TypeDecl& decl) = 0;
};

class PrettyPrinterTypeVisitor
{
  public:
  std::string virtual visit_name_type(const parser::ast::NameType& type) = 0;
  std::string virtual visit_array_type(const parser::ast::ArrayType& type) = 0;
  std::string virtual visit_record_type(const parser::ast::RecordType& type) = 0;
  std::string virtual visit_function_type(const parser::ast::FunctionType& type) = 0;
};

class PrettyPrinterVarVisitor
{
  public:
  std::string virtual visit_simple_var(const parser::ast::Var& var) = 0;
  std::string virtual visit_field_var(const parser::ast::FieldVar& var) = 0;
  std::string virtual visit_subscript_var(const parser::ast::SubscriptVar& var) = 0;
};

}; // namespace parser::ast
