#pragma once
#include "temp.hpp"
#include <cstdint>
#include <functional>
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

auto uncx(Exp&& exp) -> Cx;
auto unex(Exp&& exp) -> Ex;
auto unnx(Exp&& exp) -> Nx;

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

auto not_relop(RelOp op) -> RelOp;

struct ConstExp
{
  explicit ConstExp(int64_t v)
    : v(v)
  { }
  int64_t v;
};

struct NameExp
{
  explicit NameExp(TempGen::Label label)
    : label(std::move(label))
  { }
  TempGen::Label label;
};

struct TempExp
{
  explicit TempExp(TempGen::Temp temp)
    : temp(temp)
  { }
  TempGen::Temp temp;
};

struct BinOpExp
{
  BinOpExp(BinaryOp op, ir::Ex&& left, ir::Ex&& right);
  BinaryOp op;
  ir::Ex left;
  ir::Ex right;
};

struct MemExp
{
  explicit MemExp(ir::Ex&& address);
  ir::Ex a;
};

struct CallExp
{
  CallExp(ir::Ex&& fun, std::vector<ir::Ex>&& args);
  ir::Ex fun;
  std::vector<ir::Ex> args;
};

struct ESeqExp
{
  ESeqExp(ir::Nx&& stmt, ir::Ex&& exp);
  ir::Nx stmt;
  ir::Ex exp;
};

struct MoveStmt
{
  MoveStmt(ir::Ex&& left, ir::Ex&& right);
  ir::Ex left;
  ir::Ex right;
};

struct ExpStmt
{
  explicit ExpStmt(ir::Ex&& exp);
  ir::Ex exp;
};

struct JumpStmt
{
  JumpStmt(ir::Ex&& address, std::vector<TempGen::Label> labels);
  ir::Ex a;
  std::vector<TempGen::Label> labels;
};

struct CJumpStmt
{
  CJumpStmt(RelOp op, ir::Ex&& lexp, ir::Ex&& rexp, TempGen::Label tlabel, TempGen::Label flabel);
  RelOp op;
  ir::Ex lexp;
  ir::Ex rexp;
  TempGen::Label tlabel;
  TempGen::Label flabel;
};

struct SeqStmt
{
  SeqStmt(ir::Nx&& stm1, ir::Nx&& stm2);
  ir::Nx stm1;
  ir::Nx stm2;
};

struct LabelStmt
{
  explicit LabelStmt(TempGen::Label label)
    : label(std::move(label))
  { }
  TempGen::Label label;
};
} // namespace ir::tree