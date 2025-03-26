#pragma once

#include <functional>
#include <ir/tree.hpp>
#include <list>
#include <memory>
#include <utility>
#include <variant>

namespace ir::tree
{

class Canon {
  public:
  struct BasicBlock {
    std::list<Stmt> stmts;
  };

  std::list<Stmt> linearize(Stmt&& s)
  {
    return linear(do_stmt(std::move(s)), {});
  }

  std::pair<std::vector<BasicBlock>, TempGen::Label>
  basic_blocks(std::list<Stmt>&& l);

  std::list<Stmt> trace_schedule(std::vector<BasicBlock>&& blocks,
                                 const TempGen::Label& ldone);

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

  Stmt concat(Stmt&& s1, Stmt&& s2)
  {
    auto throw_stmt = [](const Stmt& s) {
      if(std::holds_alternative<std::unique_ptr<ExpStmt>>(s)) {
        if(std::holds_alternative<std::unique_ptr<ConstExp>>(
             std::get<std::unique_ptr<ExpStmt>>(s)->exp)) {
          // s is useless
          return true;
        }
      }
      return false;
    };

    if(throw_stmt(s1)) {
      return std::move(s2);
    }
    else if(throw_stmt(s2)) {
      return std::move(s1);
    }
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

  std::pair<Stmt, Exp> do_exp(Exp&& e);
  Stmt do_stmt(Stmt&& s);
  std::pair<Stmt, Exp>
  reorder_exp(std::list<Exp>&& el,
              std::function<Exp(std::list<Exp>&&)> build_fn);
  Stmt reorder_stmt(std::list<Exp>&& l,
                    std::function<Stmt(std::list<Exp>&&)> build_fn);
  std::pair<Stmt, std::list<Exp>> reorder(std::list<Exp>&& el);
};

} // namespace ir::tree