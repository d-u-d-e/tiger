#pragma once
#include <assert.h>
#include <memory>
#include <optional>
#include <parser/symbol.hpp>
#include <parser/visitor.hpp>
#include <utility>
#include <vector>

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
  Plus,
  Minus,
  Times,
  Divide,
  Equal,
  NotEqual,
  Less,
  LessEqual,
  Greater,
  GreaterEqual
};

inline std::string to_string(Operator op)
{
  switch(op) {
  case Operator::Plus:
    return "PlusOp";
  case Operator::Minus:
    return "MinusOp";
  case Operator::Times:
    return "TimesOp";
  case Operator::Divide:
    return "DivideOp";
  case Operator::Equal:
    return "EqualOp";
  case Operator::NotEqual:
    return "NotEqualOp";
  case Operator::Less:
    return "LessOp";
  case Operator::LessEqual:
    return "LessEqualOp";
  case Operator::Greater:
    return "GreaterOp";
  case Operator::GreaterEqual:
    return "GreaterEqualOp";
  }
  assert(false);
  std::unreachable();
};

class SimpleVar : public Variable,
                  public std::enable_shared_from_this<const SimpleVar> {
  public:
  SimpleVar(const Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_simple_var(shared_from_this());
  }

  Symbol name;
  Position position;
};

class FieldVar : public Variable,
                 public std::enable_shared_from_this<const FieldVar> {
  public:
  FieldVar(std::shared_ptr<Variable> var, const Symbol& name, Position position)
    : var(std::move(var))
    , name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_field_var(shared_from_this());
  }
  std::shared_ptr<Variable> var;
  Symbol name;
  Position position;
};

class SubscriptVar : public Variable,
                     public std::enable_shared_from_this<const SubscriptVar> {
  public:
  SubscriptVar(std::shared_ptr<Variable> var,
               std::shared_ptr<Expression> exp,
               Position position)
    : var(std::move(var))
    , exp(std::move(exp))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_subscript_var(shared_from_this());
  }
  std::shared_ptr<Variable> var;
  std::shared_ptr<Expression> exp;
  Position position;
};

class VarExp : public Expression,
               public std::enable_shared_from_this<const VarExp> {
  public:
  VarExp(std::shared_ptr<Variable> var)
    : var(std::move(var))
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_var_exp(shared_from_this());
  }
  std::shared_ptr<Variable> var;
};

class NilExp : public Expression,
               public std::enable_shared_from_this<const NilExp> {
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_nil_exp(shared_from_this());
  }
};

class IntExp : public Expression,
               public std::enable_shared_from_this<const IntExp> {
  public:
  IntExp(int value)
    : value(value)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_int_exp(shared_from_this());
  }
  int value;
};

class StringExp : public Expression,
                  public std::enable_shared_from_this<const StringExp> {
  public:
  StringExp(const std::string& value, Position position)
    : value(value)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_string_exp(shared_from_this());
  }
  std::string value;
  Position position;
};

class CallExp : public Expression,
                public std::enable_shared_from_this<const CallExp> {
  public:
  CallExp(const Symbol& func,
          std::vector<std::shared_ptr<Expression>> args,
          Position position)
    : name(func)
    , args(std::move(args))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_call_exp(shared_from_this());
  }
  Symbol name;
  std::vector<std::shared_ptr<Expression>> args;
  Position position;
};

class OpExp : public Expression,
              public std::enable_shared_from_this<const OpExp> {
  public:
  OpExp(std::shared_ptr<Expression> left,
        Operator op,
        std::shared_ptr<Expression> right,
        Position position)
    : left(std::move(left))
    , op(op)
    , right(std::move(right))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_op_exp(shared_from_this());
  }
  std::shared_ptr<Expression> left;
  Operator op;
  std::shared_ptr<Expression> right;
  Position position;
};

class _RecordField {
  public:
  _RecordField(const Symbol& name,
               std::shared_ptr<Expression> exp,
               Position position)
    : name(name)
    , exp(std::move(exp))
    , position(position)
  { }
  Symbol name;
  std::shared_ptr<Expression> exp;
  Position position;
};

class RecordExp : public Expression,
                  public std::enable_shared_from_this<const RecordExp> {
  public:
  RecordExp(const Symbol& type,
            std::vector<_RecordField> fields,
            Position position)
    : type(type)
    , fields(std::move(fields))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_record_exp(shared_from_this());
  }
  Symbol type;
  std::vector<_RecordField> fields;
  Position position;
};

class SeqExp : public Expression,
               public std::enable_shared_from_this<const SeqExp> {
  public:
  SeqExp(std::vector<std::pair<std::shared_ptr<Expression>, Position>> exps)
    : exps(std::move(exps))
  { }
  std::vector<std::pair<std::shared_ptr<Expression>, Position>> exps;
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_seq_exp(shared_from_this());
  }
};

class AssignExp : public Expression,
                  public std::enable_shared_from_this<const AssignExp> {
  public:
  AssignExp(std::shared_ptr<Variable> var,
            std::shared_ptr<Expression> exp,
            Position position)
    : var(std::move(var))
    , exp(std::move(exp))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_assign_exp(shared_from_this());
  }
  std::shared_ptr<Variable> var;
  std::shared_ptr<Expression> exp;
  Position position;
};

class IfExp : public Expression,
              public std::enable_shared_from_this<const IfExp> {
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
    return visitor.visit_if_exp(shared_from_this());
  }
  std::shared_ptr<Expression> cond;
  std::shared_ptr<Expression> then;
  std::shared_ptr<Expression> else_;
  Position position;
};

class WhileExp : public Expression,
                 public std::enable_shared_from_this<const WhileExp> {
  public:
  WhileExp(std::shared_ptr<Expression> cond,
           std::shared_ptr<Expression> body,
           Position position)
    : cond(std::move(cond))
    , body(std::move(body))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_while_exp(shared_from_this());
  }
  std::shared_ptr<Expression> cond;
  std::shared_ptr<Expression> body;
  Position position;
};

class ForExp : public Expression,
               public std::enable_shared_from_this<const ForExp> {
  public:
  ForExp(const Symbol& var,
         std::shared_ptr<Expression> low,
         std::shared_ptr<Expression> high,
         std::shared_ptr<Expression> body,
         Position position)
    : var(var)
    , low(std::move(low))
    , high(std::move(high))
    , body(std::move(body))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_for_exp(shared_from_this());
  }
  Symbol var;
  std::shared_ptr<Expression> low;
  std::shared_ptr<Expression> high;
  std::shared_ptr<Expression> body;
  Position position;
};

class BreakExp : public Expression,
                 public std::enable_shared_from_this<const BreakExp> {
  public:
  BreakExp(Position position)
    : position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_break_exp(shared_from_this());
  }
  Position position;
};

class LetExp : public Expression,
               public std::enable_shared_from_this<const LetExp> {
  public:
  LetExp(std::vector<std::shared_ptr<Declaration>> decls,
         std::shared_ptr<Expression> body,
         Position position)
    : decls(std::move(decls))
    , body(std::move(body))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_let_exp(shared_from_this());
  }
  std::vector<std::shared_ptr<Declaration>> decls;
  std::shared_ptr<Expression> body;
  Position position;
};

class ArrayExp : public Expression,
                 public std::enable_shared_from_this<const ArrayExp> {
  public:
  ArrayExp(const Symbol& type,
           std::shared_ptr<Expression> size,
           std::shared_ptr<Expression> init,
           Position position)
    : type(type)
    , size(std::move(size))
    , init(std::move(init))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_array_exp(shared_from_this());
  }
  Symbol type;
  std::shared_ptr<Expression> size;
  std::shared_ptr<Expression> init;
  Position position;
};

class VarDecl : public Declaration,
                public std::enable_shared_from_this<const VarDecl> {
  public:
  VarDecl(const Symbol& name,
          std::optional<Symbol> type,
          std::shared_ptr<Expression> init,
          Position position)
    : name(name)
    , type(type)
    , init(std::move(init))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_var_decl(shared_from_this());
  }
  Symbol name;
  std::optional<Symbol> type;
  std::shared_ptr<Expression> init;
  Position position;
};

class _TypeDecl {
  public:
  _TypeDecl(const Symbol& name, std::shared_ptr<Type> type, Position position)
    : name(name)
    , type(std::move(type))
    , position(position)
  { }
  Symbol name;
  std::shared_ptr<Type> type;
  Position position;
};

class TypeDecl : public Declaration,
                 public std::enable_shared_from_this<const TypeDecl> {
  public:
  TypeDecl(std::vector<std::shared_ptr<_TypeDecl>> decls)
    : decls(std::move(decls))
  { }
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_type_decl(shared_from_this());
  }
  std::vector<std::shared_ptr<_TypeDecl>> decls;
};

class _Field {
  public:
  _Field(const Symbol& name, const Symbol& type, Position position)
    : name(name)
    , type(type)
    , position(position)
  { }
  Symbol name;
  Symbol type;
  Position position;
};

class RecordType : public Type,
                   public std::enable_shared_from_this<const RecordType> {
  public:
  RecordType(std::vector<_Field> fields)
    : fields(std::move(fields))
  { }
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_record_type(shared_from_this());
  }
  std::vector<_Field> fields;
};

class ArrayType : public Type,
                  public std::enable_shared_from_this<const ArrayType> {
  public:
  ArrayType(const Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_array_type(shared_from_this());
  }
  Symbol name;
  Position position;
};

class NameType : public Type,
                 public std::enable_shared_from_this<const NameType> {
  public:
  NameType(const Symbol& name, Position position)
    : name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_named_type(shared_from_this());
  }
  Symbol name;
  Position position;
};

class _FuncDecl {
  public:
  _FuncDecl(const Symbol& name,
            std::vector<_Field> params,
            std::optional<Symbol> result,
            std::shared_ptr<Expression> body,
            Position position)
    : name(name)
    , params(std::move(params))
    , result(result)
    , body(std::move(body))
    , position(position)
  { }
  Symbol name;
  std::vector<_Field> params;
  std::optional<Symbol> result;
  std::shared_ptr<Expression> body;
  Position position;
};

class FuncDecl : public Declaration,
                 public std::enable_shared_from_this<const FuncDecl> {
  public:
  FuncDecl(std::vector<std::shared_ptr<_FuncDecl>> decls)
    : decls(std::move(decls))
  { }
  std::vector<std::shared_ptr<_FuncDecl>> decls;
  std::string accept(Visitor<std::string>& visitor) const
  {
    return visitor.visit_func_decl(shared_from_this());
  }
};

} // namespace ast

} // namespace parser