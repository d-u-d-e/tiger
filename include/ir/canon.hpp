#pragma once

#include "ir/visitor.hpp"
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <list>
#include <memory>
#include <utility>

namespace ir::tree
{

class Canon : public CanonExprVisitor, public CanonStmtVisitor {
  public:
  std::list<std::unique_ptr<Stmt>> linearize(std::unique_ptr<Stmt>&& s)
  {
    return linear(std::move(s), {});
  }

  private:
  std::list<std::unique_ptr<Stmt>> linear(std::unique_ptr<Stmt>&& s,
                                          std::list<std::unique_ptr<Stmt>>&& l)
  {
    auto seq = dynamic_cast<SeqStmt*>(s.get());
    if(seq) {
      return linear(std::move(seq->stm1),
                    linear(std::move(seq->stm2), std::move(l)));
    }
    else {
      std::list<std::unique_ptr<Stmt>> res;
      res.push_back(std::move(s));
      res.splice(res.end(), std::move(l));
      return res;
    }
  }

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>>
  do_exp(std::unique_ptr<Exp>&& e)
  {
    // TODO visitor
    (void)e;
    return {};
  }

  std::unique_ptr<Stmt> concat(std::unique_ptr<Stmt>&& s1,
                               std::unique_ptr<Stmt>&& s2)
  {
    return std::make_unique<SeqStmt>(std::move(s1), std::move(s2));
  }

  std::pair<std::unique_ptr<Stmt>, std::list<std::unique_ptr<Exp>>>
  reorder(std::list<std::unique_ptr<Exp>>&& el)
  {
    // base
    if(el.empty()) {
      return std::make_pair(
        std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)),
        std::list<std::unique_ptr<Exp>>{});
    }

    auto front = std::move(el.front());
    el.pop_front();

    if(auto a = dynamic_cast<CallExp*>(front.get()); a) {
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

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>>
  visit_const_exp(const ConstExp& exp) override
  {
    // TODO
    (void)exp;
    return {};
  }

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>>
  visit_name_exp(const NameExp& exp) override
  {
    // TODO
    (void)exp;
    return {};
  }

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>>
  visit_temp_exp(const TempExp& exp) override
  {
    // TODO
    (void)exp;
    return {};
  }

  std::pair<std::unique_ptr<Stmt>,
            std::unique_ptr<Exp>> virtual visit_binop_exp(const BinOpExp& exp)
    override
  {
    // TODO
    (void)exp;
    return {};
  }

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>>
  visit_mem_exp(const MemExp& exp) override
  {
    // TODO
    (void)exp;
    return {};
  }

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>>
  visit_call_exp(const CallExp& exp) override
  {
    // TODO
    (void)exp;
    return {};
  }

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>>
  visit_eseq_exp(const ESeqExp& exp) override
  {
    // TODO
    (void)exp;
    return {};
  }

  std::unique_ptr<Stmt> visit_move_stmt(const MoveStmt& stmt) override
  {
    // TODO
    (void)stmt;
    return nullptr;
  };

  std::unique_ptr<Stmt> visit_exp_stmt(const ExpStmt& stmt) override
  {
    // TODO
    (void)stmt;
    return nullptr;
  }

  std::unique_ptr<Stmt> visit_jump_stmt(const JumpStmt& stmt) override
  {
    // TODO
    (void)stmt;
    return nullptr;
  }

  std::unique_ptr<Stmt> visit_cjump_stmt(const CJumpStmt& stmt) override
  {
    // TODO
    (void)stmt;
    return nullptr;
  }

  std::unique_ptr<Stmt> visit_seq_stmt(const SeqStmt& stmt) override
  {
    // TODO
    (void)stmt;
    return nullptr;
  }

  std::unique_ptr<Stmt> visit_label_stmt(const LabelStmt& stmt) override
  {
    // TODO
    (void)stmt;
    return nullptr;
  }
};

} // namespace ir::tree