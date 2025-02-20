#pragma once
#include <memory>
#include <parser/symbol.hpp>
#include <vector>

namespace parser
{
namespace ast
{

class Expression {
  public:
  virtual ~Expression() = default;
};

class Declaration {
  public:
  virtual ~Declaration() = default;
};

class Type {
  public:
  virtual ~Type() = default;
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

class Variable {
  public:
  virtual ~Variable() = default;
};

class SimpleVar : public Variable {
  public:
  SimpleVar(const Symbol& name)
    : name(name)
  { }
  Symbol name;
};

class VarExp : public Expression {
  public:
  VarExp(std::shared_ptr<Variable> var)
    : var(std::move(var))
  { }
  std::shared_ptr<Variable> var;
};

class NilExp : public Expression { };

class IntExp : public Expression {
  public:
  IntExp(int value)
    : value(value)
  { }
  int value;
};

class StringExp : public Expression {
  public:
  StringExp(const std::string& value)
    : value(value)
  { }
  std::string value;
};

class CallExp : public Expression {
  public:
  CallExp(const Symbol& func,
          std::vector<std::shared_ptr<Expression>> args,
          int position)
    : name(name)
    , args(std::move(args))
    , position(position)
  { }
  Symbol name;
  std::vector<std::shared_ptr<Expression>> args;
  int position;
};

class OpExp : public Expression {
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
  std::shared_ptr<Expression> left;
  Operator op;
  std::shared_ptr<Expression> right;
  int position;
};

class SeqExp : public Expression {
  public:
  SeqExp(std::vector<std::shared_ptr<Expression>> exps)
    : exps(std::move(exps))
  { }
  std::vector<std::shared_ptr<Expression>> exps;
};

class AssignExp : public Expression {
  public:
  AssignExp(std::shared_ptr<Variable> left,
            std::shared_ptr<Expression> right,
            int position)
    : left(std::move(left))
    , right(std::move(right))
    , position(position)
  { }
  std::shared_ptr<Variable> left;
  std::shared_ptr<Expression> right;
  int position;
};

class IfExp : public Expression {
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
  std::shared_ptr<Expression> cond;
  std::shared_ptr<Expression> then;
  std::shared_ptr<Expression> else_;
  int position;
};

class WhileExp : public Expression {
  public:
  WhileExp(std::shared_ptr<Expression> cond,
           std::shared_ptr<Expression> body,
           int position)
    : cond(std::move(cond))
    , body(std::move(body))
    , position(position)
  { }
  std::shared_ptr<Expression> cond;
  std::shared_ptr<Expression> body;
  int position;
};

class ForExp : public Expression {
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
  Symbol var;
  std::shared_ptr<Expression> low;
  std::shared_ptr<Expression> high;
  std::shared_ptr<Expression> body;
  int position;
};

class BreakExp : public Expression {
  public:
  BreakExp(int position)
    : position(position)
  { }
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

class ArrayExp : public Expression {
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

class Field {
  public:
  Field(const Symbol& name, std::shared_ptr<Type> type, int position)
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
            std::vector<std::shared_ptr<Field>> params,
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
  std::vector<std::shared_ptr<Field>> params;
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
  RecordType(std::vector<std::shared_ptr<Field>> fields)
    : fields(std::move(fields))
  { }
  std::vector<std::shared_ptr<Field>> fields;
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