#pragma once
#include <ir/temp.hpp>
#include <memory>
#include <vector>

namespace ir
{

class Exp {
  public:
  virtual ~Exp() = default;
};

class Stmt {
  public:
  virtual ~Stmt() = default;
};

enum class BinaryOp
{
  plus,
  minus,
  mul,
  div,
  and_,
  or_,
  lshift,
  rshift,
  arshift,
  xor_
};

struct ConstExp : public Exp {
  ConstExp(int v)
    : v(v)
  { }
  int v;
};

struct NameExp : public Exp {
  NameExp(Temp::label_t label)
    : label(label)
  { }
  Temp::label_t label;
};

struct TempExp : public Exp {
  TempExp(Temp::temp_t temp)
    : temp(temp)
  { }
  Temp::temp_t temp;
};

struct BinOpExp : public Exp {
  BinOpExp(BinaryOp op, std::unique_ptr<Exp> left, std::unique_ptr<Exp> right)
    : op(op)
    , left(std::move(left))
    , right(std::move(right))
  { }
  BinaryOp op;
  std::unique_ptr<Exp> left;
  std::unique_ptr<Exp> right;
};

struct MemExp : public Exp {
  MemExp(std::unique_ptr<Exp> address)
    : a(std::move(address)){};
  std::unique_ptr<Exp> a;
};

struct CallExp : public Exp {
  CallExp(std::unique_ptr<Exp> fun, std::vector<std::unique_ptr<Exp>> args)
    : fun(std::move(fun))
    , args(std::move(args)){};
  std::unique_ptr<Exp> fun;
  std::vector<std::unique_ptr<Exp>> args;
};

struct ESeqExpr : public Exp {
  ESeqExpr(std::unique_ptr<Stmt> stmt, std::unique_ptr<Exp> exp)
    : stmt(std::move(stmt))
    , exp(std::move(exp)){};
  std::unique_ptr<Stmt> stmt;
  std::unique_ptr<Exp> exp;
};

struct MoveStmt : public Stmt {
  MoveStmt(std::unique_ptr<Exp> left, std::unique_ptr<Exp> right)
    : left(std::move(left))
    , right(std::move(right))
  { }
  std::unique_ptr<Exp> left;
  std::unique_ptr<Exp> right;
};

// TODO other stmts

} // namespace ir