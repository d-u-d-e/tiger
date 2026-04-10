#pragma once
#include "lexer/position.hpp"
#include "parser/visitor.hpp"
#include "semant/types.hpp"
#include "semant/visitor.hpp"
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <symbol.hpp>
#include <utility>
#include <vector>

namespace parser::ast
{

using Position = lexer::Position;

class Expression
{
  public:
  virtual auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string = 0;
  virtual auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result = 0;
  virtual void accept(semant::FindEscapeExprVisitor& visitor) = 0;

  Expression() = default;
  virtual ~Expression() = default;
  Expression(const Expression&) = delete;
  auto operator=(const Expression&) -> Expression& = delete;
  Expression(Expression&&) = delete;
  auto operator=(Expression&&) -> Expression& = delete;

  std::string field;
};

class Declaration
{
  public:
  virtual auto accept(PrettyPrinterDeclVisitor& visitor) const -> std::string = 0;
  virtual auto accept(semant::TypeCheckerDeclVisitor& visitor) const -> semant::types::Result = 0;
  virtual void accept(semant::FindEscapeDeclVisitor& visitor) = 0;

  Declaration() = default;
  virtual ~Declaration() = default;
  Declaration(const Declaration&) = delete;
  auto operator=(const Declaration&) -> Declaration& = delete;
  Declaration(Declaration&&) = delete;
  auto operator=(Declaration&&) -> Declaration& = delete;
  std::string field;
};

class Type
{
  public:
  virtual auto accept(PrettyPrinterTypeVisitor& visitor) const -> std::string = 0;
  virtual auto accept(semant::TypeCheckerTypeVisitor& visitor) const
    -> semant::types::SharedType = 0;

  Type() = default;
  virtual ~Type() = default;
  Type(const Type&) = delete;
  auto operator=(const Type&) -> Type& = delete;
  Type(Type&&) = delete;
  auto operator=(Type&&) -> Type& = delete;
  std::string field;
};

class Variable
{
  public:
  virtual auto accept(PrettyPrinterVarVisitor& visitor) const -> std::string = 0;
  virtual auto accept(semant::TypeCheckerVarVisitor& visitor) const -> semant::types::Result = 0;
  virtual void accept(semant::FindEscapeVarVisitor& visitor) = 0;

  virtual ~Variable() = default;
  Variable() = default;
  Variable(const Variable&) = delete;
  auto operator=(const Variable&) -> Variable& = delete;
  Variable(Variable&&) = delete;
  auto operator=(Variable&&) -> Variable& = delete;
  std::string field;
};

enum class Operator
{
  plus,
  minus,
  times,
  divide,
  equal,
  not_equal,
  less,
  less_equal,
  greater,
  greater_equal
};

inline auto to_string(Operator op) -> std::string
{
  switch(op)
  {
  case Operator::plus:
    return "PlusOp";
  case Operator::minus:
    return "MinusOp";
  case Operator::times:
    return "TimesOp";
  case Operator::divide:
    return "DivideOp";
  case Operator::equal:
    return "EqualOp";
  case Operator::not_equal:
    return "NotEqualOp";
  case Operator::less:
    return "LessOp";
  case Operator::less_equal:
    return "LessEqualOp";
  case Operator::greater:
    return "GreaterOp";
  case Operator::greater_equal:
    return "GreaterEqualOp";
  }
  assert(false);
  std::unreachable();
};

class Var : public Variable
{
  public:
  Var(Symbol name, Position position)
    : name(std::move(name))
    , position(position)
  { }
  auto accept(PrettyPrinterVarVisitor& visitor) const -> std::string override
  {
    return visitor.visit_simple_var(*this);
  }

  auto accept(semant::TypeCheckerVarVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_var(*this);
  }

  void accept(semant::FindEscapeVarVisitor& visitor) override
  {
    visitor.visit_var(*this);
  }

  Symbol name;
  Position position;
};

class FieldVar : public Variable
{
  public:
  FieldVar(std::unique_ptr<Variable> var, Symbol name, Position position)
    : var(std::move(var))
    , name(std::move(name))
    , position(position)
  { }
  auto accept(PrettyPrinterVarVisitor& visitor) const -> std::string override
  {
    return visitor.visit_field_var(*this);
  }

  auto accept(semant::TypeCheckerVarVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_field_var(*this);
  }

  void accept(semant::FindEscapeVarVisitor& visitor) override
  {
    visitor.visit_field_var(*this);
  }

  std::unique_ptr<Variable> var;
  Symbol name;
  Position position;
};

class SubscriptVar : public Variable
{
  public:
  SubscriptVar(std::unique_ptr<Variable> var, std::unique_ptr<Expression> exp, Position position)
    : var(std::move(var))
    , exp(std::move(exp))
    , position(position)
  { }
  auto accept(PrettyPrinterVarVisitor& visitor) const -> std::string override
  {
    return visitor.visit_subscript_var(*this);
  }

  auto accept(semant::TypeCheckerVarVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_subscript_var(*this);
  }

  void accept(semant::FindEscapeVarVisitor& visitor) override
  {
    visitor.visit_subscript_var(*this);
  }

  std::unique_ptr<Variable> var;
  std::unique_ptr<Expression> exp;
  Position position;
};

class VarExp : public Expression
{
  public:
  explicit VarExp(std::unique_ptr<Variable> var)
    : var(std::move(var))
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_var_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_var_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_var_exp(*this);
  }

  std::unique_ptr<Variable> var;
};

class NilExp : public Expression
{
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_nil_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_nil_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_nil_exp(*this);
  }
};

class IntExp : public Expression
{
  public:
  explicit IntExp(int64_t value)
    : value(value)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_int_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_int_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_int_exp(*this);
  }

  int64_t value;
};

class StringExp : public Expression
{
  public:
  StringExp(std::string value, Position position)
    : value(std::move(value))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_string_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_string_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_string_exp(*this);
  }

  std::string value;
  Position position;
};

class CallExp : public Expression
{
  public:
  CallExp(std::unique_ptr<Expression> callee,
          std::vector<std::unique_ptr<Expression>> args,
          Position position)
    : callee(std::move(callee))
    , args(std::move(args))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_call_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_call_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_call_exp(*this);
  }

  std::unique_ptr<Expression> callee;
  std::vector<std::unique_ptr<Expression>> args;
  Position position;
};

class OpExp : public Expression
{
  public:
  OpExp(std::unique_ptr<Expression> left,
        Operator op,
        std::unique_ptr<Expression> right,
        Position position)
    : left(std::move(left))
    , op(op)
    , right(std::move(right))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_op_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_op_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_op_exp(*this);
  }

  std::unique_ptr<Expression> left;
  Operator op;
  std::unique_ptr<Expression> right;
  Position position;
};

class RecordField_
{
  public:
  RecordField_(Symbol name, std::unique_ptr<Expression> exp, Position position)
    : name(std::move(name))
    , exp(std::move(exp))
    , position(position)
  { }
  Symbol name;
  std::unique_ptr<Expression> exp;
  Position position;
};

class RecordExp : public Expression
{
  public:
  RecordExp(Symbol type, std::vector<RecordField_> fields, Position position)
    : type(std::move(type))
    , fields(std::move(fields))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_record_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_record_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_record_exp(*this);
  }

  Symbol type;
  std::vector<RecordField_> fields;
  Position position;
};

class SeqExp : public Expression
{
  public:
  explicit SeqExp(std::vector<std::pair<std::unique_ptr<Expression>, Position>> exps)
    : exps(std::move(exps))
  { }
  std::vector<std::pair<std::unique_ptr<Expression>, Position>> exps;
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_seq_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_seq_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_seq_exp(*this);
  }
};

class AssignExp : public Expression
{
  public:
  AssignExp(std::unique_ptr<Variable> var, std::unique_ptr<Expression> exp, Position position)
    : var(std::move(var))
    , exp(std::move(exp))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_assign_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_assign_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_assign_exp(*this);
  }

  std::unique_ptr<Variable> var;
  std::unique_ptr<Expression> exp;
  Position position;
};

class IfExp : public Expression
{
  public:
  IfExp(std::shared_ptr<Expression> cond,
        std::shared_ptr<Expression> then,
        std::shared_ptr<Expression> else_,
        Position position)
    : cond(std::move(cond))
    , then(std::move(then))
    , else_(std::move(else_))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_if_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_if_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_if_exp(*this);
  }

  std::shared_ptr<Expression> cond;
  std::shared_ptr<Expression> then;
  std::shared_ptr<Expression> else_;
  Position position;
};

class WhileExp : public Expression
{
  public:
  WhileExp(std::unique_ptr<Expression> cond, std::unique_ptr<Expression> body, Position position)
    : cond(std::move(cond))
    , body(std::move(body))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_while_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_while_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_while_exp(*this);
  }

  std::unique_ptr<Expression> cond;
  std::unique_ptr<Expression> body;
  Position position;
};

class ForExp : public Expression
{
  public:
  ForExp(Symbol var,
         std::unique_ptr<Expression> low,
         std::unique_ptr<Expression> high,
         std::unique_ptr<Expression> body,
         Position position)
    : var(std::move(var))
    , low(std::move(low))
    , high(std::move(high))
    , body(std::move(body))
    , position(position)
    , escape(std::make_shared<bool>(true))
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_for_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_for_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_for_exp(*this);
  }

  Symbol var;
  std::unique_ptr<Expression> low;
  std::unique_ptr<Expression> high;
  std::unique_ptr<Expression> body;
  Position position;
  // See escape field for var declarations.
  std::shared_ptr<bool> escape;
};

class BreakExp : public Expression
{
  public:
  explicit BreakExp(Position position)
    : position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_break_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_break_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_break_exp(*this);
  }

  Position position;
};

class LetExp : public Expression
{
  public:
  LetExp(std::vector<std::unique_ptr<Declaration>> decls,
         std::unique_ptr<Expression> body,
         Position position)
    : decls(std::move(decls))
    , body(std::move(body))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_let_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_let_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_let_exp(*this);
  }

  std::vector<std::unique_ptr<Declaration>> decls;
  std::unique_ptr<Expression> body;
  Position position;
};

class ArrayExp : public Expression
{
  public:
  ArrayExp(Symbol type,
           std::unique_ptr<Expression> size,
           std::unique_ptr<Expression> init,
           Position position)
    : type(std::move(type))
    , size(std::move(size))
    , init(std::move(init))
    , position(position)
  { }
  auto accept(PrettyPrinterExprVisitor& visitor) const -> std::string override
  {
    return visitor.visit_array_exp(*this);
  }

  auto accept(semant::TypeCheckerExprVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_array_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    visitor.visit_array_exp(*this);
  }

  Symbol type;
  std::unique_ptr<Expression> size;
  std::unique_ptr<Expression> init;
  Position position;
};

class VarDecl : public Declaration
{
  public:
  VarDecl(Symbol name,
          std::optional<std::pair<Symbol, lexer::Position>> type,
          std::unique_ptr<Expression> init,
          Position position)
    : name(std::move(name))
    , type(std::move(type))
    , init(std::move(init))
    , position(position)
    , escape(std::make_shared<bool>(true))
  { }
  auto accept(PrettyPrinterDeclVisitor& visitor) const -> std::string override
  {
    return visitor.visit_var_decl(*this);
  }

  auto accept(semant::TypeCheckerDeclVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_var_decl(*this);
  }

  void accept(semant::FindEscapeDeclVisitor& visitor) override
  {
    visitor.visit_var_decl(*this);
  }

  Symbol name;
  std::optional<std::pair<Symbol, lexer::Position>> type;
  std::unique_ptr<Expression> init;
  Position position;
  /*
    The escape field tells us whether the variable is going to be used by a nested function.
    This is important to decide whether to put the variable in a register or in memory.
    Note that this is a hack, since escaping is a global nonsyntactic property. Putting it here
    means one less data structure.
  */
  std::shared_ptr<bool> escape;
};

class TypeDecl_
{
  public:
  TypeDecl_(Symbol name, std::unique_ptr<Type> type, Position position)
    : name(std::move(name))
    , type(std::move(type))
    , position(position)
  { }
  Symbol name;
  std::unique_ptr<Type> type;
  Position position;
};

class TypeDecl : public Declaration
{
  public:
  explicit TypeDecl(std::vector<std::unique_ptr<TypeDecl_>> decls)
    : decls(std::move(decls))
  { }
  auto accept(PrettyPrinterDeclVisitor& visitor) const -> std::string override
  {
    return visitor.visit_type_decl(*this);
  }

  auto accept(semant::TypeCheckerDeclVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_type_decl(*this);
  }

  void accept(semant::FindEscapeDeclVisitor& visitor) override
  {
    visitor.visit_type_decl(*this);
  }

  std::vector<std::unique_ptr<TypeDecl_>> decls;
};

class Field_
{
  public:
  Field_(Symbol name, Symbol type, Position position)
    : name(std::move(name))
    , type(std::move(type))
    , position(position)
    , escape(std::make_shared<bool>(true))
  { }
  Symbol name;
  Symbol type;
  Position position;
  // The escape field is important for function parameters. It can be ignored for record fields.
  // See escape field for var declarations.
  std::shared_ptr<bool> escape;
};

class RecordType : public Type
{
  public:
  explicit RecordType(std::vector<Field_> fields)
    : fields(std::move(fields))
  { }
  auto accept(PrettyPrinterTypeVisitor& visitor) const -> std::string override
  {
    return visitor.visit_record_type(*this);
  }

  auto accept(semant::TypeCheckerTypeVisitor& visitor) const -> semant::types::SharedType override
  {
    return visitor.visit_record_type(*this);
  }

  std::vector<Field_> fields;
};

class ArrayType : public Type
{
  public:
  ArrayType(Symbol name, Position position)
    : name(std::move(name))
    , position(position)
  { }
  auto accept(PrettyPrinterTypeVisitor& visitor) const -> std::string override
  {
    return visitor.visit_array_type(*this);
  }

  auto accept(semant::TypeCheckerTypeVisitor& visitor) const -> semant::types::SharedType override
  {
    return visitor.visit_array_type(*this);
  }

  Symbol name;
  Position position;
};

class NameType : public Type
{
  public:
  NameType(Symbol name, Position position)
    : name(std::move(name))
    , position(position)
  { }
  auto accept(PrettyPrinterTypeVisitor& visitor) const -> std::string override
  {
    return visitor.visit_name_type(*this);
  }

  auto accept(semant::TypeCheckerTypeVisitor& visitor) const -> semant::types::SharedType override
  {
    return visitor.visit_name_type(*this);
  }

  Symbol name;
  Position position;
};

class FunctionType : public Type
{
  public:
  FunctionType(std::vector<std::unique_ptr<Type>> arg_types,
               std::unique_ptr<Type> ret_type,
               Position position)
    : arg_types(std::move(arg_types))
    , ret_type(std::move(ret_type))
    , position(position)
  { }
  auto accept(PrettyPrinterTypeVisitor& visitor) const -> std::string override
  {
    return visitor.visit_function_type(*this);
  }

  auto accept(semant::TypeCheckerTypeVisitor& visitor) const -> semant::types::SharedType override
  {
    return visitor.visit_func_type(*this);
  }
  std::vector<std::unique_ptr<Type>> arg_types;
  std::unique_ptr<Type> ret_type;
  Position position;
};

class FuncDecl_
{
  public:
  FuncDecl_(Symbol name,
            std::vector<Field_> params,
            std::optional<std::pair<Symbol, lexer::Position>> result,
            std::unique_ptr<Expression> body,
            Position position)
    : name(std::move(name))
    , params(std::move(params))
    , result(std::move(result))
    , body(std::move(body))
    , position(position)
    , escape(std::make_shared<bool>(true))
  { }
  Symbol name;
  std::vector<Field_> params;
  std::optional<std::pair<Symbol, lexer::Position>> result;
  std::unique_ptr<Expression> body;
  Position position;

  /*
    The escape field tells us whether the variable is going to be used by a nested function.
    This is important to decide whether to put the variable in a register or in memory.
    Note that this is a hack, since escaping is a global nonsyntactic property. Putting it here
    means one less data structure.
  */
  std::shared_ptr<bool> escape;
};

class FuncDecl : public Declaration
{
  public:
  explicit FuncDecl(std::vector<std::unique_ptr<FuncDecl_>> decls)
    : decls(std::move(decls))
  { }
  std::vector<std::unique_ptr<FuncDecl_>> decls;
  auto accept(PrettyPrinterDeclVisitor& visitor) const -> std::string override
  {
    return visitor.visit_func_decl(*this);
  }

  auto accept(semant::TypeCheckerDeclVisitor& visitor) const -> semant::types::Result override
  {
    return visitor.visit_func_decl(*this);
  }

  void accept(semant::FindEscapeDeclVisitor& visitor) override
  {
    visitor.visit_func_decl(*this);
  }
};

} // namespace parser::ast
