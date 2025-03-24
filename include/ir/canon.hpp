#pragma once

#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <list>
#include <memory>
#include <utility>

namespace ir::tree
{

class Canon {
  public:
  std::list<Stmt> linearize(Stmt&& s)
  {
    return linear(std::move(s), {});
  }

  private:
  std::list<Stmt> linear(Stmt&& s, std::list<Stmt>&& l)
  {
    if(std::holds_alternative<std::unique_ptr<SeqStmt>>(s)) {
      auto seq = std::move(std::get<std::unique_ptr<SeqStmt>>(s));
      return linear(std::move(seq->stm1),
                    linear(std::move(seq->stm2), std::move(l)));
    }
    else {
      l.push_front(std::move(s));
      return l;
    }
  }

  std::pair<Stmt, Exp> do_exp(Exp&& e)
  {
    // TODO visitor
    (void)e;
    return {};
  }

  Stmt concat(Stmt&& s1, Stmt&& s2)
  {
    return std::make_unique<SeqStmt>(std::move(s1), std::move(s2));
  }

  std::pair<Stmt, std::list<Exp>> reorder(std::list<Exp>&& el)
  {
    // base
    if(el.empty()) {
      return std::make_pair(
        std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)),
        std::list<Exp>{});
    }

    auto front = std::move(el.front());
    el.pop_front();

    if(std::holds_alternative<std::unique_ptr<CallExp>>(front)) {
      // all call expressions should put the result in a temporary
      auto t = TempGen::new_temp();
      el.push_front(std::make_unique<ESeqExp>(
        std::make_unique<MoveStmt>(std::make_unique<TempExp>(t),
                                   std::move(front)),
        std::make_unique<TempExp>(t)));

      return reorder(std::move(el));
    }

    auto [stmt, e] = do_exp(std::move(front));

    // reorder the rest
    auto [stmt_, el_] = reorder(std::move(el));

    // TODO: if stmt_ can commute, we can avoid moving to a temporary
    auto temp = ir::TempGen::new_temp();
    auto a = concat(std::move(stmt),
                    std::make_unique<MoveStmt>(std::make_unique<TempExp>(temp),
                                               std::move(e)));

    el_.push_front(std::make_unique<TempExp>(temp));
    return {concat(std::move(a), std::move(stmt_)), std::move(el_)};
  }
};

} // namespace ir::tree