#pragma once
#include <cstdint>
#include <functional>
#include <ir/temp.hpp>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace ir::tree
{
struct ConstExp;
struct NameExp;
struct TempExp;
struct BinOpExp;
struct MemExp;
struct CallExp;
struct ESeqExp;
struct MoveStmt;
struct ExpStmt;
struct JumpStmt;
struct CJumpStmt;
struct SeqStmt;
struct LabelStmt;

using Exp = std::variant<std::unique_ptr<ConstExp>,
                         std::unique_ptr<NameExp>,
                         std::unique_ptr<TempExp>,
                         std::unique_ptr<BinOpExp>,
                         std::unique_ptr<MemExp>,
                         std::unique_ptr<CallExp>,
                         std::unique_ptr<ESeqExp>>;

using Stmt = std::variant<std::unique_ptr<MoveStmt>,
                          std::unique_ptr<ExpStmt>,
                          std::unique_ptr<JumpStmt>,
                          std::unique_ptr<CJumpStmt>,
                          std::unique_ptr<SeqStmt>,
                          std::unique_ptr<LabelStmt>>;
} // namespace ir::tree

namespace ir
{
using Ex = tree::Exp;
using Nx = tree::Stmt;
using Cx = std::move_only_function<tree::Stmt(TempGen::Label, TempGen::Label)>;
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

RelOp not_relop(RelOp op);

struct ConstExp {
  ConstExp(int64_t v)
    : v(v)
  { }
  int64_t v;
};

struct NameExp {
  NameExp(TempGen::Label label)
    : label(label)
  { }
  TempGen::Label label;
};

struct TempExp {
  TempExp(TempGen::Temp temp)
    : temp(temp)
  { }
  TempGen::Temp temp;
};

struct BinOpExp {
  BinOpExp(BinaryOp op, ir::Ex&& left, ir::Ex&& right)
    : op(op)
    , left(std::move(left))
    , right(std::move(right))
  { }
  BinaryOp op;
  ir::Ex left;
  ir::Ex right;
};

struct MemExp {
  MemExp(ir::Ex&& address)
    : a(std::move(address)){};
  ir::Ex a;
};

struct CallExp {
  CallExp(ir::Ex&& fun, std::vector<ir::Ex>&& args)
    : fun(std::move(fun))
    , args(std::move(args)){};
  ir::Ex fun;
  std::vector<ir::Ex> args;
};

struct ESeqExp {
  ESeqExp(ir::Nx&& stmt, ir::Ex&& exp)
    : stmt(std::move(stmt))
    , exp(std::move(exp)){};
  ir::Nx stmt;
  ir::Ex exp;
};

struct MoveStmt {
  MoveStmt(ir::Ex&& left, ir::Ex&& right)
    : left(std::move(left))
    , right(std::move(right))
  { }
  ir::Ex left;
  ir::Ex right;
};

struct ExpStmt {
  ExpStmt(ir::Ex&& exp)
    : exp(std::move(exp))
  { }
  ir::Ex exp;
};

struct JumpStmt {
  JumpStmt(ir::Ex&& address, std::vector<TempGen::Label> labels)
    : a(std::move(address))
    , labels(std::move(labels))
  { }
  ir::Ex a;
  std::vector<TempGen::Label> labels;
};

struct CJumpStmt {
  CJumpStmt(RelOp op, ir::Ex&& lexp, ir::Ex&& rexp, TempGen::Label tlabel, TempGen::Label flabel)
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
};

struct SeqStmt {
  SeqStmt(ir::Nx&& stm1, ir::Nx&& stm2)
    : stm1(std::move(stm1))
    , stm2(std::move(stm2))
  { }
  ir::Nx stm1;
  ir::Nx stm2;
};

struct LabelStmt {
  LabelStmt(TempGen::Label label)
    : label(label)
  { }
  TempGen::Label label;
};

} // namespace ir::tree