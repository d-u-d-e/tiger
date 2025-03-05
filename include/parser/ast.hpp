#pragma once
#include <assert.h>
#include <memory>
#include <optional>
#include <parser/visitor.hpp>
#include <symbol.hpp>
#include <utility>
#include <vector>
#include <lexer/position.hpp>

namespace parser
{
namespace ast
{

using Position = lexer::Position;

class Expression {
  public:
  virtual std::string accept(Visitor<std::string>& visitor) const = 0;
  virtual ~Expression() = default;
  std::string field;
};

class Declaration {
  public:
  virtual std::string accept(Visitor<std::string>& visitor) const = 0;
  virtual ~Declaration() = default;
  std::string field;
};

class Type {
  public:
  virtual std::string accept(Visitor<std::string>& visitor) const = 0;
  virtual ~Type() = default;
  std::string field;
};

class Variable {
  public:
  virtual std::string accept(Visitor<std::string>& visitor) const = 0;
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
  switch(op) {
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

class SimpleVar : public Variable {
  public:
  SimpleVar(const symbol::Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_simple_var(*this);
  }

  symbol::Symbol name;
  Position position;
};

class FieldVar : public Variable {
  public:
  FieldVar(std::unique_ptr<Variable> var,
           const symbol::Symbol& name,
           Position position)
    : var(std::move(var))
    , name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_field_var(*this);
  }
  std::unique_ptr<Variable> var;
  symbol::Symbol name;
  Position position;
};

class SubscriptVar : public Variable {
  public:
  SubscriptVar(std::unique_ptr<Variable> var,
               std::unique_ptr<Expression> exp,
               Position position)
    : var(std::move(var))
    , exp(std::move(exp))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_subscript_var(*this);
  }
  std::unique_ptr<Variable> var;
  std::unique_ptr<Expression> exp;
  Position position;
};

class VarExp : public Expression {
  public:
  VarExp(std::unique_ptr<Variable> var)
    : var(std::move(var))
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_var_exp(*this);
  }
  std::unique_ptr<Variable> var;
};

class NilExp : public Expression {
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_nil_exp(*this);
  }
};

class IntExp : public Expression {
  public:
  IntExp(int value)
    : value(value)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_int_exp(*this);
  }
  int value;
};

class StringExp : public Expression {
  public:
  StringExp(const std::string& value, Position position)
    : value(value)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_string_exp(*this);
  }
  std::string value;
  Position position;
};

class CallExp : public Expression {
  public:
  CallExp(const symbol::Symbol& func,
          std::vector<std::unique_ptr<Expression>> args,
          Position position)
    : name(func)
    , args(std::move(args))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_call_exp(*this);
  }
  symbol::Symbol name;
  std::vector<std::unique_ptr<Expression>> args;
  Position position;
};

class OpExp : public Expression {
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
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_op_exp(*this);
  }
  std::unique_ptr<Expression> left;
  Operator op;
  std::unique_ptr<Expression> right;
  Position position;
};

class _RecordField {
  public:
  _RecordField(const symbol::Symbol& name,
               std::unique_ptr<Expression> exp,
               Position position)
    : name(name)
    , exp(std::move(exp))
    , position(position)
  { }
  symbol::Symbol name;
  std::unique_ptr<Expression> exp;
  Position position;
};

class RecordExp : public Expression {
  public:
  RecordExp(const symbol::Symbol& type,
            std::vector<_RecordField> fields,
            Position position)
    : type(type)
    , fields(std::move(fields))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_record_exp(*this);
  }
  symbol::Symbol type;
  std::vector<_RecordField> fields;
  Position position;
};

class SeqExp : public Expression {
  public:
  SeqExp(std::vector<std::pair<std::unique_ptr<Expression>, Position>> exps)
    : exps(std::move(exps))
  { }
  std::vector<std::pair<std::unique_ptr<Expression>, Position>> exps;
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_seq_exp(*this);
  }
};

class AssignExp : public Expression {
  public:
  AssignExp(std::unique_ptr<Variable> var,
            std::unique_ptr<Expression> exp,
            Position position)
    : var(std::move(var))
    , exp(std::move(exp))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_assign_exp(*this);
  }
  std::unique_ptr<Variable> var;
  std::unique_ptr<Expression> exp;
  Position position;
};

class IfExp : public Expression {
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
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_if_exp(*this);
  }
  std::shared_ptr<Expression> cond;
  std::shared_ptr<Expression> then;
  std::shared_ptr<Expression> else_;
  Position position;
};

class WhileExp : public Expression {
  public:
  WhileExp(std::unique_ptr<Expression> cond,
           std::unique_ptr<Expression> body,
           Position position)
    : cond(std::move(cond))
    , body(std::move(body))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_while_exp(*this);
  }
  std::unique_ptr<Expression> cond;
  std::unique_ptr<Expression> body;
  Position position;
};

class ForExp : public Expression {
  public:
  ForExp(const symbol::Symbol& var,
         std::unique_ptr<Expression> low,
         std::unique_ptr<Expression> high,
         std::unique_ptr<Expression> body,
         Position position)
    : var(var)
    , low(std::move(low))
    , high(std::move(high))
    , body(std::move(body))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_for_exp(*this);
  }
  symbol::Symbol var;
  std::unique_ptr<Expression> low;
  std::unique_ptr<Expression> high;
  std::unique_ptr<Expression> body;
  Position position;
};

class BreakExp : public Expression {
  public:
  BreakExp(Position position)
    : position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_break_exp(*this);
  }
  Position position;
};

class LetExp : public Expression {
  public:
  LetExp(std::vector<std::unique_ptr<Declaration>> decls,
         std::unique_ptr<Expression> body,
         Position position)
    : decls(std::move(decls))
    , body(std::move(body))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_let_exp(*this);
  }
  std::vector<std::unique_ptr<Declaration>> decls;
  std::unique_ptr<Expression> body;
  Position position;
};

class ArrayExp : public Expression {
  public:
  ArrayExp(const symbol::Symbol& type,
           std::unique_ptr<Expression> size,
           std::unique_ptr<Expression> init,
           Position position)
    : type(type)
    , size(std::move(size))
    , init(std::move(init))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_array_exp(*this);
  }
  symbol::Symbol type;
  std::unique_ptr<Expression> size;
  std::unique_ptr<Expression> init;
  Position position;
};

class VarDecl : public Declaration {
  public:
  VarDecl(const symbol::Symbol& name,
          std::optional<symbol::Symbol> type,
          std::unique_ptr<Expression> init,
          Position position)
    : name(name)
    , type(type)
    , init(std::move(init))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_var_decl(*this);
  }
  symbol::Symbol name;
  std::optional<symbol::Symbol> type;
  std::unique_ptr<Expression> init;
  Position position;
};

class _TypeDecl {
  public:
  _TypeDecl(const symbol::Symbol& name,
            std::unique_ptr<Type> type,
            Position position)
    : name(name)
    , type(std::move(type))
    , position(position)
  { }
  symbol::Symbol name;
  std::unique_ptr<Type> type;
  Position position;
};

class TypeDecl : public Declaration {
  public:
  TypeDecl(std::vector<std::unique_ptr<_TypeDecl>> decls)
    : decls(std::move(decls))
  { }
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_type_decl(*this);
  }
  std::vector<std::unique_ptr<_TypeDecl>> decls;
};

class _Field {
  public:
  _Field(const symbol::Symbol& name,
         const symbol::Symbol& type,
         Position position)
    : name(name)
    , type(type)
    , position(position)
  { }
  symbol::Symbol name;
  symbol::Symbol type;
  Position position;
};

class RecordType : public Type {
  public:
  RecordType(std::vector<_Field> fields)
    : fields(std::move(fields))
  { }
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_record_type(*this);
  }
  std::vector<_Field> fields;
};

class ArrayType : public Type {
  public:
  ArrayType(const symbol::Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_array_type(*this);
  }
  symbol::Symbol name;
  Position position;
};

class NameType : public Type {
  public:
  NameType(const symbol::Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_named_type(*this);
  }
  symbol::Symbol name;
  Position position;
};

class _FuncDecl {
  public:
  _FuncDecl(const symbol::Symbol& name,
            std::vector<_Field> params,
            std::optional<symbol::Symbol> result,
            std::unique_ptr<Expression> body,
            Position position)
    : name(name)
    , params(std::move(params))
    , result(result)
    , body(std::move(body))
    , position(position)
  { }
  symbol::Symbol name;
  std::vector<_Field> params;
  std::optional<symbol::Symbol> result;
  std::unique_ptr<Expression> body;
  Position position;
};

class FuncDecl : public Declaration {
  public:
  FuncDecl(std::vector<std::unique_ptr<_FuncDecl>> decls)
    : decls(std::move(decls))
  { }
  std::vector<std::unique_ptr<_FuncDecl>> decls;
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_func_decl(*this);
  }
};

} // namespace ast

} // namespace parser