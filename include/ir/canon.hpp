#pragma once

#include <functional>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <list>
#include <memory>
#include <utility>
#include <variant>

namespace ir::tree
{

class Canon {
  public:
  std::list<Stmt> linearize(Stmt&& s)
  {
    return linear(std::move(s), {});
  }

  // do_exp
  std::pair<Stmt, Exp> operator()(std::unique_ptr<ConstExp> e);
  std::pair<Stmt, Exp> operator()(std::unique_ptr<NameExp> e);
  std::pair<Stmt, Exp> operator()(std::unique_ptr<TempExp> e);
  std::pair<Stmt, Exp> operator()(std::unique_ptr<BinOpExp> e);
  std::pair<Stmt, Exp> operator()(std::unique_ptr<MemExp> e);
  std::pair<Stmt, Exp> operator()(std::unique_ptr<CallExp> e);
  std::pair<Stmt, Exp> operator()(std::unique_ptr<ESeqExp> e);

  // do_stmt
  Stmt operator()(std::unique_ptr<ExpStmt> s);
  Stmt operator()(std::unique_ptr<MoveStmt> s);
  Stmt operator()(std::unique_ptr<JumpStmt> s);
  Stmt operator()(std::unique_ptr<CJumpStmt> s);
  Stmt operator()(std::unique_ptr<SeqStmt> s);
  Stmt operator()(std::unique_ptr<LabelStmt> s);

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

  std::pair<Stmt, Exp> do_exp(Exp&& e);
  Stmt do_stmt(Stmt&& s);

  Stmt concat(Stmt&& s1, Stmt&& s2)
  {
    return std::make_unique<SeqStmt>(std::move(s1), std::move(s2));
  }

  bool commute(const Stmt& stmt, const Exp& exp)
  {
    if(std::holds_alternative<std::unique_ptr<ExpStmt>>(stmt)) {
      auto& exp_stmt = std::get<std::unique_ptr<ExpStmt>>(stmt);
      if(std::holds_alternative<std::unique_ptr<ConstExp>>(exp_stmt->exp)) {
        // an expression statement containing a constant commute with any expression
        return true;
      }
    }
    else if(std::holds_alternative<std::unique_ptr<NameExp>>(exp)) {
      // a name expression commutes with any statement
      return true;
    }
    else if(std::holds_alternative<std::unique_ptr<ConstExp>>(exp)) {
      // a constant expression commutes with any statement
      return true;
    }
    return false;
  }

  std::pair<Stmt, Exp>
  reorder_exp(std::list<Exp>&& el,
              std::function<Exp(std::list<Exp>&&)> build_fn);

  Stmt reorder_stmt(std::list<Exp>&& l,
                    std::function<Stmt(std::list<Exp>&&)> build_fn);

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

    // if stmt_ can commute, we can avoid moving to a temporary
    if(commute(stmt_, e)) {
      el_.push_front(std::move(e));
      return {concat(std::move(stmt), std::move(stmt_)), std::move(el_)};
    }
    else {
      auto temp = ir::TempGen::new_temp();
      auto a = concat(std::move(stmt),
                      std::make_unique<MoveStmt>(
                        std::make_unique<TempExp>(temp), std::move(e)));

      el_.push_front(std::make_unique<TempExp>(temp));
      return {concat(std::move(a), std::move(stmt_)), std::move(el_)};
    }
  }

  // do_exp
};

} // namespace ir::tree