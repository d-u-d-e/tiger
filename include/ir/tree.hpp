#pragma once
#include <functional>
#include <ir/temp.hpp>
#include <ir/visitor.hpp>
#include <memory>
#include <vector>

namespace ir
{

class Exp {
  public:
  virtual ~Exp() = default;
  virtual std::string accept(PrettyPrinterExprVisitor& visitor) = 0;
};

class Stmt {
  public:
  virtual ~Stmt() = default;
  virtual std::string accept(PrettyPrinterStmtVisitor& visitor) = 0;
};

using ex_t = std::unique_ptr<ir::Exp>;
using nx_t = std::unique_ptr<ir::Stmt>;
using cx_t =
  std::function<std::unique_ptr<ir::Stmt>(Temp::label_t, Temp::label_t)>;
using exp_t = std::variant<std::monostate, ex_t, nx_t, cx_t>;

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

enum class RelOp
{
  eq,
  ne,
  lt,
  gt,
  le,
  ge,
  ult,
  ule,
  ugt,
  uge
};

struct ConstExp : public Exp {
  ConstExp(int v)
    : v(v)
  { }
  int v;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_const_exp(*this);
  }
};

struct NameExp : public Exp {
  NameExp(Temp::label_t label)
    : label(label)
  { }
  Temp::label_t label;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_name_exp(*this);
  }
};

struct TempExp : public Exp {
  TempExp(Temp::temp_t temp)
    : temp(temp)
  { }
  Temp::temp_t temp;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_temp_exp(*this);
  }
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
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_binop_exp(*this);
  }
};

struct MemExp : public Exp {
  MemExp(std::unique_ptr<Exp> address)
    : a(std::move(address)){};
  std::unique_ptr<Exp> a;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_mem_exp(*this);
  }
};

struct CallExp : public Exp {
  CallExp(std::unique_ptr<Exp> fun, std::vector<std::unique_ptr<Exp>> args)
    : fun(std::move(fun))
    , args(std::move(args)){};
  std::unique_ptr<Exp> fun;
  std::vector<std::unique_ptr<Exp>> args;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_call_exp(*this);
  }
};

struct ESeqExp : public Exp {
  ESeqExp(std::unique_ptr<Stmt> stmt, std::unique_ptr<Exp> exp)
    : stmt(std::move(stmt))
    , exp(std::move(exp)){};
  std::unique_ptr<Stmt> stmt;
  std::unique_ptr<Exp> exp;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_eseq_exp(*this);
  }
};

struct MoveStmt : public Stmt {
  MoveStmt(std::unique_ptr<Exp> left, std::unique_ptr<Exp> right)
    : left(std::move(left))
    , right(std::move(right))
  { }
  std::unique_ptr<Exp> left;
  std::unique_ptr<Exp> right;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_move_stmt(*this);
  }
};

struct ExpStmt : public Stmt {
  ExpStmt(std::unique_ptr<Exp> exp)
    : exp(std::move(exp))
  { }
  std::unique_ptr<Exp> exp;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_exp_stmt(*this);
  }
};

struct JumpStmt : public Stmt {
  JumpStmt(std::unique_ptr<Exp> address, std::vector<Temp::label_t> labels)
    : a(std::move(a))
    , labels(std::move(labels))
  { }
  std::unique_ptr<Exp> a;
  std::vector<Temp::label_t> labels;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_jump_stmt(*this);
  }
};

struct CJumpStmt : public Stmt {
  CJumpStmt(RelOp op,
            std::unique_ptr<Exp> lexp,
            std::unique_ptr<Exp> rexp,
            Temp::label_t tlabel,
            Temp::label_t flabel)
    : op(op)
    , lexp(std::move(lexp))
    , rexp(std::move(rexp))
    , tlabel(tlabel)
    , flabel(flabel)
  { }
  RelOp op;
  std::unique_ptr<Exp> lexp;
  std::unique_ptr<Exp> rexp;
  Temp::label_t tlabel;
  Temp::label_t flabel;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_cjump_stmt(*this);
  }
};

struct SeqStmt : public Stmt {
  SeqStmt(std::unique_ptr<Stmt> stm1, std::unique_ptr<Stmt> stm2)
    : stm1(std::move(stm1))
    , stm2(std::move(stm2))
  { }
  std::unique_ptr<Stmt> stm1;
  std::unique_ptr<Stmt> stm2;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_seq_stmt(*this);
  }
};

struct LabelStmt : public Stmt {
  LabelStmt(Temp::label_t label)
    : label(label)
  { }
  Temp::label_t label;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_label_stmt(*this);
  }
};

} // namespace ir