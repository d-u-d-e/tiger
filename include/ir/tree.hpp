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
using cx_t = std::move_only_function<std::unique_ptr<ir::Stmt>(Temp::label_t,
                                                               Temp::label_t)>;
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
  ConstExp(size_t v)
    : v(v)
  { }
  size_t v;
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
  BinOpExp(BinaryOp op, ex_t&& left, ex_t&& right)
    : op(op)
    , left(std::move(left))
    , right(std::move(right))
  { }
  BinaryOp op;
  ex_t left;
  ex_t right;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_binop_exp(*this);
  }
};

struct MemExp : public Exp {
  MemExp(ex_t&& address)
    : a(std::move(address)){};
  ex_t a;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_mem_exp(*this);
  }
};

struct CallExp : public Exp {
  CallExp(ex_t&& fun, std::vector<ex_t>&& args)
    : fun(std::move(fun))
    , args(std::move(args)){};
  ex_t fun;
  std::vector<ex_t> args;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_call_exp(*this);
  }
};

struct ESeqExp : public Exp {
  ESeqExp(nx_t&& stmt, ex_t&& exp)
    : stmt(std::move(stmt))
    , exp(std::move(exp)){};
  nx_t stmt;
  ex_t exp;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_eseq_exp(*this);
  }
};

struct MoveStmt : public Stmt {
  MoveStmt(ex_t&& left, ex_t&& right)
    : left(std::move(left))
    , right(std::move(right))
  { }
  ex_t left;
  ex_t right;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_move_stmt(*this);
  }
};

struct ExpStmt : public Stmt {
  ExpStmt(ex_t&& exp)
    : exp(std::move(exp))
  { }
  ex_t exp;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_exp_stmt(*this);
  }
};

struct JumpStmt : public Stmt {
  JumpStmt(ex_t&& address, std::vector<Temp::label_t> labels)
    : a(std::move(address))
    , labels(std::move(labels))
  { }
  ex_t a;
  std::vector<Temp::label_t> labels;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_jump_stmt(*this);
  }
};

struct CJumpStmt : public Stmt {
  CJumpStmt(
    RelOp op, ex_t&& lexp, ex_t&& rexp, Temp::label_t tlabel, Temp::label_t flabel)
    : op(op)
    , lexp(std::move(lexp))
    , rexp(std::move(rexp))
    , tlabel(tlabel)
    , flabel(flabel)
  { }
  RelOp op;
  ex_t lexp;
  ex_t rexp;
  Temp::label_t tlabel;
  Temp::label_t flabel;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_cjump_stmt(*this);
  }
};

struct SeqStmt : public Stmt {
  SeqStmt(nx_t&& stm1, nx_t&& stm2)
    : stm1(std::move(stm1))
    , stm2(std::move(stm2))
  { }
  nx_t stm1;
  nx_t stm2;
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