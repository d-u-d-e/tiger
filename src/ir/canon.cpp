#include <algorithm>
#include <cassert>
#include <functional>
#include <ir/canon.hpp>
#include <ir/tree.hpp>
#include <list>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

namespace ir::tree
{

std::pair<Stmt, Exp>
Canon::reorder_exp(std::list<Exp>&& l,
                   std::function<Exp(std::list<Exp>&&)> build_fn)
{
  auto [stmt, el] = reorder(std::move(l));
  return std::make_pair(std::move(stmt), build_fn(std::move(el)));
}

Stmt Canon::reorder_stmt(std::list<Exp>&& l,
                         std::function<Stmt(std::list<Exp>&&)> build_fn)
{
  auto [stmt, el] = reorder(std::move(l));
  return concat(std::move(stmt), build_fn(std::move(el)));
}

std::pair<Stmt, Exp> Canon::do_exp(Exp&& exp)
{
  // This mimics do_exp from Appel's solutions
  return std::visit(*this, std::move(exp));
}

Stmt Canon::do_stmt(Stmt&& s)
{
  // This mimics do_exp from Appel's solutions
  return std::visit(*this, std::move(s));
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<ConstExp> e)
{
  // nothing to do
  return std::make_pair<Stmt, Exp>(
    std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)), std::move(e));
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<NameExp> e)
{
  // nothing to do
  return std::make_pair<Stmt, Exp>(
    std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)), std::move(e));
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<TempExp> e)
{
  // nothing to do
  return std::make_pair<Stmt, Exp>(
    std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)), std::move(e));
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<BinOpExp> e)
{
  std::list<Exp> subexps;
  subexps.push_back(std::move(e->left));
  subexps.push_back(std::move(e->right));
  return reorder_exp(std::move(subexps), [op = e->op](std::list<Exp>&& l) {
    auto left = std::move(l.front());
    l.pop_front();
    auto right = std::move(l.front());
    l.pop_front();
    return std::make_unique<BinOpExp>(op, std::move(left), std::move(right));
  });
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<MemExp> e)
{
  std::list<Exp> subexps;
  subexps.push_back(std::move(e->a));

  return reorder_exp(std::move(subexps), [](std::list<Exp>&& l) {
    auto a = std::move(l.front());
    l.pop_front();
    return std::make_unique<MemExp>(std::move(a));
  });
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<CallExp> e)
{
  std::list<Exp> subexps;
  subexps.push_back(std::move(e->fun));
  std::move(e->args.begin(), e->args.end(), subexps.end());

  return reorder_exp(std::move(subexps), [](std::list<Exp>&& l) {
    auto f = std::move(l.front());
    l.pop_front();
    std::vector<Exp> args;
    std::move(l.begin(), l.end(), args.begin());
    return std::make_unique<CallExp>(std::move(f), std::move(args));
  });
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<ESeqExp> e)
{
  Stmt s = do_stmt(std::move(e->stmt));
  auto [stmt, e_] = do_exp(std::move(e->exp));
  return std::make_pair(concat(std::move(s), std::move(stmt)), std::move(e_));
}

Stmt Canon::operator()(std::unique_ptr<ExpStmt> s)
{
  std::list<Exp> subexps;
  if(std::holds_alternative<std::unique_ptr<CallExp>>(s->exp)) {
    // just a call expression which discards the return value
    auto ce = std::move(std::get<std::unique_ptr<CallExp>>(s->exp));
    std::list<Exp> subexps;
    subexps.push_back(std::move(ce->fun));
    std::move(ce->args.begin(), ce->args.begin(), subexps.end());

    return reorder_stmt(std::move(subexps), [](std::list<Exp>&& l) {
      auto f = std::move(l.front());
      l.pop_front();
      std::vector<Exp> args;
      std::move(l.begin(), l.end(), args.begin());
      return std::make_unique<ExpStmt>(
        std::make_unique<CallExp>(std::move(f), std::move(args)));
    });
  }

  subexps.push_back(std::move(s->exp));
  return reorder_stmt(std::move(subexps), [](std::list<Exp>&& l) {
    auto e = std::move(l.front());
    l.pop_front();
    return std::make_unique<ExpStmt>(std::move(e));
  });
}

Stmt Canon::operator()(std::unique_ptr<JumpStmt> s)
{
  std::list<Exp> subexps;
  subexps.push_back(std::move(s->a));
  return reorder_stmt(std::move(subexps),
                      [labs = s->labels](std::list<Exp>&& l) {
                        auto a = std::move(l.front());
                        l.pop_front();
                        return std::make_unique<JumpStmt>(std::move(a), labs);
                      });
}

Stmt Canon::operator()(std::unique_ptr<CJumpStmt> s)
{
  std::list<Exp> subexps;
  subexps.push_back(std::move(s->lexp));
  subexps.push_back(std::move(s->rexp));
  return reorder_stmt(
    std::move(subexps),
    [tlab = s->tlabel, flab = s->flabel, op = s->op](std::list<Exp>&& l) {
      auto lexp = std::move(l.front());
      l.pop_front();
      auto rexp = std::move(l.front());
      l.pop_front();
      return std::make_unique<CJumpStmt>(
        op, std::move(lexp), std::move(rexp), tlab, flab);
    });
}

Stmt Canon::operator()(std::unique_ptr<SeqStmt> s)
{
  return concat(do_stmt(std::move(s->stm1)), do_stmt(std::move(s->stm1)));
}

Stmt Canon::operator()(std::unique_ptr<LabelStmt> s)
{
  return Stmt(std::move(s));
}

Stmt Canon::operator()(std::unique_ptr<MoveStmt> s)
{

  /*
    | do_stm(T.MOVE(T.TEMP t,b)) = 
	       reorder_stm([b],fn[b]=>T.MOVE(T.TEMP t,b))
    | do_stm(T.MOVE(T.MEM e,b)) = 
	       reorder_stm([e,b],fn[e,b]=>T.MOVE(T.MEM e,b))
    | do_stm(T.MOVE(T.ESEQ(s,e),b)) = 
	       do_stm(T.SEQ(s,T.MOVE(e,b)))
  */

  if(std::holds_alternative<std::unique_ptr<TempExp>>(s->left)) {
    // moving to a temporary TODO
    return Stmt(std::move(s));
  }
  else if(std::holds_alternative<std::unique_ptr<MemExp>>(s->left)) {
    // moving to a memory location
    auto& mem_exp = std::get<std::unique_ptr<MemExp>>(s->left);
    std::list<Exp> subexps;
    subexps.push_front(std::move(mem_exp->a));
    subexps.push_front(std::move(s->right));
    return reorder_stmt(std::move(subexps), [](std::list<Exp>&& l) {
      auto a = std::move(l.front());
      l.pop_front();
      auto right = std::move(l.front());
      l.pop_front();
      return std::make_unique<MoveStmt>(std::make_unique<MemExp>(std::move(a)),
                                        std::move(right));
    });
  }
  assert(std::holds_alternative<std::unique_ptr<ESeqExp>>(s->left));
  auto& eseq = std::get<std::unique_ptr<ESeqExp>>(s->left);
  return do_stmt(std::make_unique<SeqStmt>(
    std::move(eseq->stmt),
    std::make_unique<MoveStmt>(std::move(eseq->exp), std::move(s->right))));
}

} // namespace ir::tree