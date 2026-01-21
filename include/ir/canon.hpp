#pragma once
#include "ir/tree.hpp"
#include "temp.hpp"
#include <list>
#include <vector>

namespace ir::tree
{

class Canon
{
  public:
  struct BasicBlock
  {
    std::list<Stmt> stmts;
    bool visited{false};
  };

  std::list<Stmt> linearize(Stmt&& s)
  {
    return linear(do_stmt(std::move(s)), {});
  }

  static std::pair<std::vector<BasicBlock>, TempGen::Label> basic_blocks(std::list<Stmt>&& l);
  static std::list<Stmt> trace_schedule(std::vector<BasicBlock>&& blocks,
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
  std::pair<Stmt, Exp> do_exp(Exp&& e);
  Stmt do_stmt(Stmt&& s);
  std::pair<Stmt, Exp> reorder_exp(std::list<Exp>&& el,
                                   std::function<Exp(std::list<Exp>&&)> build_fn);
  Stmt reorder_stmt(std::list<Exp>&& l, std::function<Stmt(std::list<Exp>&&)> build_fn);
  std::pair<Stmt, std::list<Exp>> reorder(std::list<Exp>&& el);
  static bool commute(const Stmt& stmt, const Exp& exp);
  static Stmt concat(Stmt&& s1, Stmt&& s2);
  static std::list<Stmt> linear(Stmt&& s, std::list<Stmt>&& l);
};
} // namespace ir::tree