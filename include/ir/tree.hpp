#pragma once
#include <cstddef>
#include <functional>
#include <ir/temp.hpp>
#include <ir/visitor.hpp>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace ir::tree
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
} // namespace ir::tree

namespace ir
{
// expressions
using Ex = std::unique_ptr<tree::Exp>;

// statements which do not produce values
using Nx = std::unique_ptr<tree::Stmt>;

// expressions that evaluate to boolean are better represented by a conditional jump
using Cx = std::move_only_function<std::unique_ptr<tree::Stmt>(TempGen::Label,
                                                               TempGen::Label)>;
using Exp = std::variant<std::monostate, Ex, Nx, Cx>;
} // namespace ir

namespace ir::tree
{

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
  NameExp(TempGen::Label label)
    : label(label)
  { }
  TempGen::Label label;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_name_exp(*this);
  }
};

struct TempExp : public Exp {
  TempExp(TempGen::Temp temp)
    : temp(temp)
  { }
  TempGen::Temp temp;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_temp_exp(*this);
  }
};

struct BinOpExp : public Exp {
  BinOpExp(BinaryOp op, ir::Ex&& left, ir::Ex&& right)
    : op(op)
    , left(std::move(left))
    , right(std::move(right))
  { }
  BinaryOp op;
  ir::Ex left;
  ir::Ex right;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_binop_exp(*this);
  }
};

struct MemExp : public Exp {
  MemExp(ir::Ex&& address)
    : a(std::move(address)){};
  ir::Ex a;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_mem_exp(*this);
  }
};

struct CallExp : public Exp {
  CallExp(ir::Ex&& fun, std::vector<ir::Ex>&& args)
    : fun(std::move(fun))
    , args(std::move(args)){};
  ir::Ex fun;
  std::vector<ir::Ex> args;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_call_exp(*this);
  }
};

struct ESeqExp : public Exp {
  ESeqExp(ir::Nx&& stmt, ir::Ex&& exp)
    : stmt(std::move(stmt))
    , exp(std::move(exp)){};
  ir::Nx stmt;
  ir::Ex exp;
  std::string accept(PrettyPrinterExprVisitor& visitor)
  {
    return visitor.visit_eseq_exp(*this);
  }
};

struct MoveStmt : public Stmt {
  MoveStmt(ir::Ex&& left, ir::Ex&& right)
    : left(std::move(left))
    , right(std::move(right))
  { }
  ir::Ex left;
  ir::Ex right;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_move_stmt(*this);
  }
};

struct ExpStmt : public Stmt {
  ExpStmt(ir::Ex&& exp)
    : exp(std::move(exp))
  { }
  ir::Ex exp;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_exp_stmt(*this);
  }
};

struct JumpStmt : public Stmt {
  JumpStmt(ir::Ex&& address, std::vector<TempGen::Label> labels)
    : a(std::move(address))
    , labels(std::move(labels))
  { }
  ir::Ex a;
  std::vector<TempGen::Label> labels;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_jump_stmt(*this);
  }
};

struct CJumpStmt : public Stmt {
  CJumpStmt(RelOp op,
            ir::Ex&& lexp,
            ir::Ex&& rexp,
            TempGen::Label tlabel,
            TempGen::Label flabel)
    : op(op)
    , lexp(std::move(lexp))
    , rexp(std::move(rexp))
    , tlabel(tlabel)
    , flabel(flabel)
  { }
  RelOp op;
  ir::Ex lexp;
  ir::Ex rexp;
  TempGen::Label tlabel;
  TempGen::Label flabel;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_cjump_stmt(*this);
  }
};

struct SeqStmt : public Stmt {
  SeqStmt(ir::Nx&& stm1, ir::Nx&& stm2)
    : stm1(std::move(stm1))
    , stm2(std::move(stm2))
  { }
  ir::Nx stm1;
  ir::Nx stm2;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_seq_stmt(*this);
  }
};

struct LabelStmt : public Stmt {
  LabelStmt(TempGen::Label label)
    : label(label)
  { }
  TempGen::Label label;
  std::string accept(PrettyPrinterStmtVisitor& visitor)
  {
    return visitor.visit_label_stmt(*this);
  }
};

} // namespace ir::tree