#pragma once
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
  PrettyPrinterExprVisitor() = default;
  virtual ~PrettyPrinterExprVisitor() = default;
  PrettyPrinterExprVisitor(const PrettyPrinterExprVisitor&) = default;
  auto operator=(const PrettyPrinterExprVisitor&) -> PrettyPrinterExprVisitor& = default;
  PrettyPrinterExprVisitor(PrettyPrinterExprVisitor&&) = default;
  auto operator=(PrettyPrinterExprVisitor&&) -> PrettyPrinterExprVisitor& = default;

  auto virtual visit_string_exp(const parser::ast::StringExp& exp) -> std::string = 0;
  auto virtual visit_assign_exp(const parser::ast::AssignExp& exp) -> std::string = 0;
  auto virtual visit_op_exp(const parser::ast::OpExp& exp) -> std::string = 0;
  auto virtual visit_int_exp(const parser::ast::IntExp& exp) -> std::string = 0;
  auto virtual visit_var_exp(const parser::ast::VarExp& exp) -> std::string = 0;
  auto virtual visit_seq_exp(const parser::ast::SeqExp& exp) -> std::string = 0;
  auto virtual visit_array_exp(const parser::ast::ArrayExp& exp) -> std::string = 0;
  auto virtual visit_nil_exp(const parser::ast::NilExp& exp) -> std::string = 0;
  auto virtual visit_record_exp(const parser::ast::RecordExp& exp) -> std::string = 0;
  auto virtual visit_if_exp(const parser::ast::IfExp& exp) -> std::string = 0;
  auto virtual visit_break_exp(const parser::ast::BreakExp& exp) -> std::string = 0;
  auto virtual visit_while_exp(const parser::ast::WhileExp& exp) -> std::string = 0;
  auto virtual visit_for_exp(const parser::ast::ForExp& exp) -> std::string = 0;
  auto virtual visit_call_exp(const parser::ast::CallExp& exp) -> std::string = 0;
  auto virtual visit_let_exp(const parser::ast::LetExp& exp) -> std::string = 0;
};

class PrettyPrinterDeclVisitor
{
  public:
  PrettyPrinterDeclVisitor() = default;
  virtual ~PrettyPrinterDeclVisitor() = default;
  PrettyPrinterDeclVisitor(const PrettyPrinterDeclVisitor&) = default;
  auto operator=(const PrettyPrinterDeclVisitor&) -> PrettyPrinterDeclVisitor& = default;
  PrettyPrinterDeclVisitor(PrettyPrinterDeclVisitor&&) = default;
  auto operator=(PrettyPrinterDeclVisitor&&) -> PrettyPrinterDeclVisitor& = default;

  auto virtual visit_func_decl(const parser::ast::FuncDecl& decl) -> std::string = 0;
  auto virtual visit_var_decl(const parser::ast::VarDecl& decl) -> std::string = 0;
  auto virtual visit_type_decl(const parser::ast::TypeDecl& decl) -> std::string = 0;
};

class PrettyPrinterTypeVisitor
{
  public:
  PrettyPrinterTypeVisitor() = default;
  virtual ~PrettyPrinterTypeVisitor() = default;
  PrettyPrinterTypeVisitor(const PrettyPrinterTypeVisitor&) = default;
  auto operator=(const PrettyPrinterTypeVisitor&) -> PrettyPrinterTypeVisitor& = default;
  PrettyPrinterTypeVisitor(PrettyPrinterTypeVisitor&&) = default;
  auto operator=(PrettyPrinterTypeVisitor&&) -> PrettyPrinterTypeVisitor& = default;

  auto virtual visit_name_type(const parser::ast::NameType& type) -> std::string = 0;
  auto virtual visit_array_type(const parser::ast::ArrayType& type) -> std::string = 0;
  auto virtual visit_record_type(const parser::ast::RecordType& type) -> std::string = 0;
  auto virtual visit_function_type(const parser::ast::FunctionType& type) -> std::string = 0;
};

class PrettyPrinterVarVisitor
{
  public:
  PrettyPrinterVarVisitor() = default;
  virtual ~PrettyPrinterVarVisitor() = default;
  PrettyPrinterVarVisitor(const PrettyPrinterVarVisitor&) = default;
  auto operator=(const PrettyPrinterVarVisitor&) -> PrettyPrinterVarVisitor& = default;
  PrettyPrinterVarVisitor(PrettyPrinterVarVisitor&&) = default;
  auto operator=(PrettyPrinterVarVisitor&&) -> PrettyPrinterVarVisitor& = default;

  auto virtual visit_simple_var(const parser::ast::Var& var) -> std::string = 0;
  auto virtual visit_field_var(const parser::ast::FieldVar& var) -> std::string = 0;
  auto virtual visit_subscript_var(const parser::ast::SubscriptVar& var) -> std::string = 0;
};

}; // namespace parser::ast
