#include "ir/tree.hpp"
#include <cassert>
#include <utility>

namespace ir
{

namespace tree
{
BinOpExp::BinOpExp(BinaryOp op, ir::Ex&& left, ir::Ex&& right)
  : op(op)
  , left(std::move(left))
  , right(std::move(right))
{ }

MemExp::MemExp(ir::Ex&& address)
  : a(std::move(address))
{ }

CallExp::CallExp(ir::Ex&& fun, std::vector<ir::Ex>&& args)
  : fun(std::move(fun))
  , args(std::move(args))
{ }

ESeqExp::ESeqExp(ir::Nx&& stmt, ir::Ex&& exp)
  : stmt(std::move(stmt))
  , exp(std::move(exp))
{ }

MoveStmt::MoveStmt(ir::Ex&& left, ir::Ex&& right)
  : left(std::move(left))
  , right(std::move(right))
{ }

ExpStmt::ExpStmt(ir::Ex&& exp)
  : exp(std::move(exp))
{ }

JumpStmt::JumpStmt(ir::Ex&& address, std::vector<TempGen::Label> labels)
  : a(std::move(address))
  , labels(std::move(labels))
{ }

CJumpStmt::CJumpStmt(
  RelOp op, ir::Ex&& lexp, ir::Ex&& rexp, TempGen::Label tlabel, TempGen::Label flabel)
  : op(op)
  , lexp(std::move(lexp))
  , rexp(std::move(rexp))
  , tlabel(std::move(tlabel))
  , flabel(std::move(flabel))
{ }

SeqStmt::SeqStmt(ir::Nx&& stm1, ir::Nx&& stm2)
  : stm1(std::move(stm1))
  , stm2(std::move(stm2))
{ }

auto not_relop(RelOp op) -> RelOp
{
  switch(op)
  {
  case RelOp::eq:
    return RelOp::ne;
  case RelOp::ne:
    return RelOp::eq;
  case RelOp::lt:
    return RelOp::ge;
  case RelOp::gt:
    return RelOp::le;
  case RelOp::le:
    return RelOp::gt;
  case RelOp::ge:
    return RelOp::lt;
  case RelOp::ult:
    return RelOp::uge;
  case RelOp::ule:
    return RelOp::ugt;
  case RelOp::ugt:
    return RelOp::ule;
  case RelOp::uge:
    return RelOp::ult;
  default:
    assert(false);
  }
  std::unreachable();
}
} // namespace tree

auto unex(Exp&& exp) -> Ex
{
  // unex(nx) is just ESeqExp(nx, 0)
  // unex(ex) is just ex
  // unex(cx) is:
  // ESeq(SeqStmt[MoveStmt(temp, 1), cx(t, f), LabelStmt(f), MoveStmt(temp, 0), LabelStmt(t)], temp)

  if(std::holds_alternative<Ex>(exp))
  {
    return std::move(std::get<Ex>(exp));
  }
  else if(std::holds_alternative<Nx>(exp))
  {
    return std::make_unique<tree::ESeqExp>(std::move(std::get<Nx>(exp)),
                                           std::make_unique<tree::ConstExp>(0));
  }
  else if(std::holds_alternative<Cx>(exp))
  {
    auto cx = std::move(std::get<Cx>(exp));
    auto temp = TempGen::new_temp();
    auto tlab = TempGen::new_label();
    auto flab = TempGen::new_label();
    auto seq = std::make_unique<tree::SeqStmt>(
      std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(temp),
                                       std::make_unique<tree::ConstExp>(1)),
      cx(tlab, flab));
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(flab));
    seq = std::make_unique<tree::SeqStmt>(
      std::move(seq),
      std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(temp),
                                       std::make_unique<tree::ConstExp>(0)));
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(tlab));
    return std::make_unique<tree::ESeqExp>(std::move(seq), std::make_unique<tree::TempExp>(temp));
  }
  assert(false);
  std::unreachable();
}

auto unnx(Exp&& exp) -> Nx
{
  // unnx(nx) is just nx
  // unnx(ex) is a ExpStmt(ex)
  // unnx(cx) is Seq[cx(t, f), LabelStmt(t), LabelStmt(f)]

  if(std::holds_alternative<Ex>(exp))
  {
    return std::make_unique<tree::ExpStmt>(std::move(std::get<Ex>(exp)));
  }
  else if(std::holds_alternative<Nx>(exp))
  {
    return std::move(std::get<Nx>(exp));
  }
  else if(std::holds_alternative<Cx>(exp))
  {
    auto tlab = TempGen::new_label();
    auto flab = TempGen::new_label();
    auto seq = std::make_unique<tree::SeqStmt>(std::get<Cx>(exp)(tlab, flab),
                                               std::make_unique<tree::LabelStmt>(tlab));
    return std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(flab));
  }
  assert(false);
  std::unreachable();
}

auto uncx(Exp&& exp) -> Cx
{
  // uncx(nx) should not occur in a valid program
  // uncx(ex) is: (t, f) -> CJumpStmt(eq, ex, 0, f, t)
  // uncx(cx) is just cx

  if(std::holds_alternative<Ex>(exp))
  {
    return [e = std::move(exp)](const TempGen::Label& t, const TempGen::Label& f) mutable {
      return std::make_unique<tree::CJumpStmt>(
        tree::RelOp::eq, std::move(std::get<Ex>(e)), std::make_unique<tree::ConstExp>(0), f, t);
    };
  }
  else if(std::holds_alternative<Cx>(exp))
  {
    return std::move(std::get<Cx>(exp));
  }

  assert(false);
  std::unreachable();
}
} // namespace ir