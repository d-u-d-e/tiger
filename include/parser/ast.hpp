#pragma once
#include <assert.h>
#include <memory>
#include <parser/symbol.hpp>
#include <parser/visitor.hpp>
#include <utility>
#include <vector>

namespace parser
{
namespace ast
{

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
  SimpleVar(const Symbol& name, int position)
    : name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_simple_var(shared_from_this());
  }

  Symbol name;
  int position;
};

class FieldVar : public Variable,
                 public std::enable_shared_from_this<const FieldVar> {
  public:
  FieldVar(std::shared_ptr<Variable> var, const Symbol& name, int position)
    : var(var)
    , name(name)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_field_var(shared_from_this());
  }
  std::shared_ptr<Variable> var;
  Symbol name;
  int position;
};

class SubscriptVar : public Variable,
                     public std::enable_shared_from_this<const SubscriptVar> {
  public:
  SubscriptVar(std::shared_ptr<Variable> var,
               std::shared_ptr<Expression> exp,
               int position)
    : var(var)
    , exp(exp)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_subscript_var(shared_from_this());
  }
  std::shared_ptr<Variable> var;
  std::shared_ptr<Expression> exp;
  int position;
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
  StringExp(const std::string& value, int position)
    : value(value)
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_string_exp(shared_from_this());
  }
  std::string value;
  int position;
};

class CallExp : public Expression,
                public std::enable_shared_from_this<const CallExp> {
  public:
  CallExp(const Symbol& func,
          std::vector<std::shared_ptr<Expression>> args,
          int position)
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
  int position;
};

class OpExp : public Expression,
              public std::enable_shared_from_this<const OpExp> {
  public:
  OpExp(std::shared_ptr<Expression> left,
        Operator op,
        std::shared_ptr<Expression> right,
        int position)
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
  int position;
};

class RecordField {
  public:
  RecordField(const Symbol& name, std::shared_ptr<Expression> exp, int position)
    : name(name)
    , exp(std::move(exp))
    , position(position)
  { }
  Symbol name;
  std::shared_ptr<Expression> exp;
  int position;
};

class RecordExp : public Expression,
                  public std::enable_shared_from_this<const RecordExp> {
  public:
  RecordExp(const Symbol& type, std::vector<RecordField> fields, int position)
    : type(type)
    , fields(std::move(fields))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_record_exp(shared_from_this());
  }
  Symbol type;
  std::vector<RecordField> fields;
  int position;
};

class SeqExp : public Expression,
               public std::enable_shared_from_this<const SeqExp> {
  public:
  SeqExp(std::vector<std::pair<std::shared_ptr<Expression>, int>> exps)
    : exps(std::move(exps))
  { }
  std::vector<std::pair<std::shared_ptr<Expression>, int>> exps;
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_seq_exp(shared_from_this());
  }
};

class AssignExp : public Expression,
                  public std::enable_shared_from_this<const AssignExp> {
  public:
  AssignExp(std::shared_ptr<Variable> left,
            std::shared_ptr<Expression> right,
            int position)
    : left(std::move(left))
    , right(std::move(right))
    , position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_assign_exp(shared_from_this());
  }
  std::shared_ptr<Variable> left;
  std::shared_ptr<Expression> right;
  int position;
};

class IfExp : public Expression,
              public std::enable_shared_from_this<const IfExp> {
  public:
  IfExp(std::shared_ptr<Expression> cond,
        std::shared_ptr<Expression> then,
        std::shared_ptr<Expression> else_,
        int position)
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
  int position;
};

class WhileExp : public Expression,
                 public std::enable_shared_from_this<const WhileExp> {
  public:
  WhileExp(std::shared_ptr<Expression> cond,
           std::shared_ptr<Expression> body,
           int position)
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
  int position;
};

class ForExp : public Expression,
               public std::enable_shared_from_this<const ForExp> {
  public:
  ForExp(const Symbol& var,
         std::shared_ptr<Expression> low,
         std::shared_ptr<Expression> high,
         std::shared_ptr<Expression> body,
         int position)
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
  int position;
};

class BreakExp : public Expression,
                 public std::enable_shared_from_this<const BreakExp> {
  public:
  BreakExp(int position)
    : position(position)
  { }
  std::string accept(Visitor<std::string>& visitor) const override
  {
    return visitor.visit_break_exp(shared_from_this());
  }
  int position;
};

class LetExp : public Expression {
  public:
  LetExp(std::vector<std::shared_ptr<Declaration>> decls,
         std::shared_ptr<Expression> body,
         int position)
    : decls(std::move(decls))
    , body(std::move(body))
    , position(position)
  { }
  std::vector<std::shared_ptr<Declaration>> decls;
  std::shared_ptr<Expression> body;
  int position;
};

class ArrayExp : public Expression,
                 public std::enable_shared_from_this<const ArrayExp> {
  public:
  ArrayExp(const Symbol& type,
           std::shared_ptr<Expression> size,
           std::shared_ptr<Expression> init,
           int position)
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
  int position;
};

class VarDecl : public Declaration {
  public:
  VarDecl(const Symbol& name,
          const Symbol& type,
          std::shared_ptr<Expression> init,
          int position)
    : name(name)
    , type(std::move(type))
    , init(std::move(init))
    , position(position)
  { }
  Symbol name;
  Symbol type;
  std::shared_ptr<Expression> init;
  int position;
};

class _TypeDecl {
  public:
  _TypeDecl(const Symbol& name, std::shared_ptr<Type> type, int position)
    : name(name)
    , type(std::move(type))
    , position(position)
  { }
  Symbol name;
  std::shared_ptr<Type> type;
  int position;
};

class TypeDecl : public Declaration {
  public:
  TypeDecl(std::vector<std::shared_ptr<_TypeDecl>> decls)
    : decls(std::move(decls))
  { }
  std::vector<std::shared_ptr<_TypeDecl>> decls;
};

class DeclField {
  public:
  DeclField(const Symbol& name, std::shared_ptr<Type> type, int position)
    : name(name)
    , type(std::move(type))
    , position(position)
  { }
  Symbol name;
  std::shared_ptr<Type> type;
  int position;
};

class _FuncDecl {
  public:
  _FuncDecl(const Symbol& name,
            std::vector<std::shared_ptr<DeclField>> params,
            const Symbol& result,
            std::shared_ptr<Expression> body,
            int position)
    : name(name)
    , params(std::move(params))
    , result(result)
    , body(std::move(body))
    , position(position)
  { }
  Symbol name;
  std::vector<std::shared_ptr<DeclField>> params;
  Symbol result;
  std::shared_ptr<Expression> body;
  int position;
};

class FuncDecl : public Declaration {
  public:
  FuncDecl(std::vector<std::shared_ptr<_FuncDecl>> decls)
    : decls(std::move(decls))
  { }
  std::vector<std::shared_ptr<_FuncDecl>> decls;
};

class RecordType : public Type {
  public:
  RecordType(std::vector<std::shared_ptr<DeclField>> fields)
    : fields(std::move(fields))
  { }
  std::vector<std::shared_ptr<DeclField>> fields;
};

class ArrayType : public Type {
  public:
  ArrayType(const Symbol& name)
    : name(name)
  { }
  Symbol name;
};

} // namespace ast

} // namespace parser