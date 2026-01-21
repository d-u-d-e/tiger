#include "ir/tree.hpp"
#include <cassert>
#include <utility>

namespace ir
{

namespace tree
{
RelOp not_relop(RelOp op)
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

Ex unex(Exp&& exp)
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

Nx unnx(Exp&& exp)
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

Cx uncx(Exp&& exp)
{
  // uncx(nx) should not occur in a valid program
  // uncx(ex) is: (t, f) -> CJumpStmt(eq, ex, 0, f, t)
  // uncx(cx) is just cx

  if(std::holds_alternative<Ex>(exp))
  {
    return [e = std::move(exp)](TempGen::Label t, TempGen::Label f) mutable {
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