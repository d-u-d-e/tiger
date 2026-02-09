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
  virtual std::string accept(PrettyPrinterExprVisitor& visitor) const = 0;
  virtual semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const = 0;
  virtual void accept(semant::FindEscapeExprVisitor& visitor) = 0;
  virtual ~Expression() = default;
  std::string field;
};

class Declaration
{
  public:
  virtual std::string accept(PrettyPrinterDeclVisitor& visitor) const = 0;
  virtual semant::types::Result accept(semant::TypeCheckerDeclVisitor& visitor) const = 0;
  virtual void accept(semant::FindEscapeDeclVisitor& visitor) = 0;
  virtual ~Declaration() = default;
  std::string field;
};

class Type
{
  public:
  virtual std::string accept(PrettyPrinterTypeVisitor& visitor) const = 0;
  virtual semant::types::SharedType accept(semant::TypeCheckerTypeVisitor& visitor) const = 0;
  virtual ~Type() = default;
  std::string field;
};

class Variable
{
  public:
  virtual std::string accept(PrettyPrinterVarVisitor& visitor) const = 0;
  virtual semant::types::Result accept(semant::TypeCheckerVarVisitor& visitor) const = 0;
  virtual void accept(semant::FindEscapeVarVisitor& visitor) = 0;
  virtual ~Variable() = default;
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

inline std::string to_string(Operator op)
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
  Var(const Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(PrettyPrinterVarVisitor& visitor) const override
  {
    return visitor.visit_simple_var(*this);
  }

  semant::types::Result accept(semant::TypeCheckerVarVisitor& visitor) const override
  {
    return visitor.visit_var(*this);
  }

  void accept(semant::FindEscapeVarVisitor& visitor) override
  {
    return visitor.visit_var(*this);
  }

  Symbol name;
  Position position;
};

class FieldVar : public Variable
{
  public:
  FieldVar(std::unique_ptr<Variable> var, const Symbol& name, Position position)
    : var(std::move(var))
    , name(name)
    , position(position)
  { }
  std::string accept(PrettyPrinterVarVisitor& visitor) const override
  {
    return visitor.visit_field_var(*this);
  }

  semant::types::Result accept(semant::TypeCheckerVarVisitor& visitor) const override
  {
    return visitor.visit_field_var(*this);
  }

  void accept(semant::FindEscapeVarVisitor& visitor) override
  {
    return visitor.visit_field_var(*this);
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
  std::string accept(PrettyPrinterVarVisitor& visitor) const override
  {
    return visitor.visit_subscript_var(*this);
  }

  semant::types::Result accept(semant::TypeCheckerVarVisitor& visitor) const override
  {
    return visitor.visit_subscript_var(*this);
  }

  void accept(semant::FindEscapeVarVisitor& visitor) override
  {
    return visitor.visit_subscript_var(*this);
  }

  std::unique_ptr<Variable> var;
  std::unique_ptr<Expression> exp;
  Position position;
};

class VarExp : public Expression
{
  public:
  VarExp(std::unique_ptr<Variable> var)
    : var(std::move(var))
  { }
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_var_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_var_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_var_exp(*this);
  }

  std::unique_ptr<Variable> var;
};

class NilExp : public Expression
{
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_nil_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_nil_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_nil_exp(*this);
  }
};

class IntExp : public Expression
{
  public:
  IntExp(int64_t value)
    : value(value)
  { }
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_int_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_int_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_int_exp(*this);
  }

  int64_t value;
};

class StringExp : public Expression
{
  public:
  StringExp(const std::string& value, Position position)
    : value(value)
    , position(position)
  { }
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_string_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_string_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_string_exp(*this);
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
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_call_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_call_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_call_exp(*this);
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
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_op_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_op_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_op_exp(*this);
  }

  std::unique_ptr<Expression> left;
  Operator op;
  std::unique_ptr<Expression> right;
  Position position;
};

class _RecordField
{
  public:
  _RecordField(const Symbol& name, std::unique_ptr<Expression> exp, Position position)
    : name(name)
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
  RecordExp(const Symbol& type, std::vector<_RecordField> fields, Position position)
    : type(type)
    , fields(std::move(fields))
    , position(position)
  { }
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_record_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_record_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_record_exp(*this);
  }

  Symbol type;
  std::vector<_RecordField> fields;
  Position position;
};

class SeqExp : public Expression
{
  public:
  SeqExp(std::vector<std::pair<std::unique_ptr<Expression>, Position>> exps)
    : exps(std::move(exps))
  { }
  std::vector<std::pair<std::unique_ptr<Expression>, Position>> exps;
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_seq_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_seq_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_seq_exp(*this);
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
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_assign_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_assign_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_assign_exp(*this);
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
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_if_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_if_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_if_exp(*this);
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
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_while_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_while_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_while_exp(*this);
  }

  std::unique_ptr<Expression> cond;
  std::unique_ptr<Expression> body;
  Position position;
};

class ForExp : public Expression
{
  public:
  ForExp(const Symbol& var,
         std::unique_ptr<Expression> low,
         std::unique_ptr<Expression> high,
         std::unique_ptr<Expression> body,
         Position position)
    : var(var)
    , low(std::move(low))
    , high(std::move(high))
    , body(std::move(body))
    , position(position)
    , escape(std::make_shared<bool>(true))
  { }
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_for_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_for_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_for_exp(*this);
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
  BreakExp(Position position)
    : position(position)
  { }
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_break_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_break_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_break_exp(*this);
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
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_let_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_let_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_let_exp(*this);
  }

  std::vector<std::unique_ptr<Declaration>> decls;
  std::unique_ptr<Expression> body;
  Position position;
};

class ArrayExp : public Expression
{
  public:
  ArrayExp(const Symbol& type,
           std::unique_ptr<Expression> size,
           std::unique_ptr<Expression> init,
           Position position)
    : type(type)
    , size(std::move(size))
    , init(std::move(init))
    , position(position)
  { }
  std::string accept(PrettyPrinterExprVisitor& visitor) const override
  {
    return visitor.visit_array_exp(*this);
  }

  semant::types::Result accept(semant::TypeCheckerExprVisitor& visitor) const override
  {
    return visitor.visit_array_exp(*this);
  }

  void accept(semant::FindEscapeExprVisitor& visitor) override
  {
    return visitor.visit_array_exp(*this);
  }

  Symbol type;
  std::unique_ptr<Expression> size;
  std::unique_ptr<Expression> init;
  Position position;
};

class VarDecl : public Declaration
{
  public:
  VarDecl(const Symbol& name,
          std::optional<std::pair<Symbol, lexer::Position>> type,
          std::unique_ptr<Expression> init,
          Position position)
    : name(name)
    , type(type)
    , init(std::move(init))
    , position(position)
    , escape(std::make_shared<bool>(true))
  { }
  std::string accept(PrettyPrinterDeclVisitor& visitor) const override
  {
    return visitor.visit_var_decl(*this);
  }

  semant::types::Result accept(semant::TypeCheckerDeclVisitor& visitor) const override
  {
    return visitor.visit_var_decl(*this);
  }

  void accept(semant::FindEscapeDeclVisitor& visitor) override
  {
    return visitor.visit_var_decl(*this);
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

class _TypeDecl
{
  public:
  _TypeDecl(const Symbol& name, std::unique_ptr<Type> type, Position position)
    : name(name)
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
  TypeDecl(std::vector<std::unique_ptr<_TypeDecl>> decls)
    : decls(std::move(decls))
  { }
  std::string accept(PrettyPrinterDeclVisitor& visitor) const override
  {
    return visitor.visit_type_decl(*this);
  }

  semant::types::Result accept(semant::TypeCheckerDeclVisitor& visitor) const override
  {
    return visitor.visit_type_decl(*this);
  }

  void accept(semant::FindEscapeDeclVisitor& visitor) override
  {
    return visitor.visit_type_decl(*this);
  }

  std::vector<std::unique_ptr<_TypeDecl>> decls;
};

class _Field
{
  public:
  _Field(const Symbol& name, const Symbol& type, Position position)
    : name(name)
    , type(type)
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
  RecordType(std::vector<_Field> fields)
    : fields(std::move(fields))
  { }
  std::string accept(PrettyPrinterTypeVisitor& visitor) const override
  {
    return visitor.visit_record_type(*this);
  }

  semant::types::SharedType accept(semant::TypeCheckerTypeVisitor& visitor) const override
  {
    return visitor.visit_record_type(*this);
  }

  std::vector<_Field> fields;
};

class ArrayType : public Type
{
  public:
  ArrayType(const Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(PrettyPrinterTypeVisitor& visitor) const override
  {
    return visitor.visit_array_type(*this);
  }

  semant::types::SharedType accept(semant::TypeCheckerTypeVisitor& visitor) const override
  {
    return visitor.visit_array_type(*this);
  }

  Symbol name;
  Position position;
};

class NameType : public Type
{
  public:
  NameType(const Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(PrettyPrinterTypeVisitor& visitor) const override
  {
    return visitor.visit_name_type(*this);
  }

  semant::types::SharedType accept(semant::TypeCheckerTypeVisitor& visitor) const override
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
  std::string accept(PrettyPrinterTypeVisitor& visitor) const override
  {
    return visitor.visit_function_type(*this);
  }

  semant::types::SharedType accept(semant::TypeCheckerTypeVisitor& visitor) const override
  {
    return visitor.visit_func_type(*this);
  }
  std::vector<std::unique_ptr<Type>> arg_types;
  std::unique_ptr<Type> ret_type;
  Position position;
};

class _FuncDecl
{
  public:
  _FuncDecl(const Symbol& name,
            std::vector<_Field> params,
            std::optional<std::pair<Symbol, lexer::Position>> result,
            std::unique_ptr<Expression> body,
            Position position)
    : name(name)
    , params(std::move(params))
    , result(result)
    , body(std::move(body))
    , position(position)
    , escape(std::make_shared<bool>(true))
  { }
  Symbol name;
  std::vector<_Field> params;
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
  FuncDecl(std::vector<std::unique_ptr<_FuncDecl>> decls)
    : decls(std::move(decls))
  { }
  std::vector<std::unique_ptr<_FuncDecl>> decls;
  std::string accept(PrettyPrinterDeclVisitor& visitor) const override
  {
    return visitor.visit_func_decl(*this);
  }

  semant::types::Result accept(semant::TypeCheckerDeclVisitor& visitor) const override
  {
    return visitor.visit_func_decl(*this);
  }

  void accept(semant::FindEscapeDeclVisitor& visitor) override
  {
    return visitor.visit_func_decl(*this);
  }
};

} // namespace parser::ast
