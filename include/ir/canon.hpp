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

  auto linearize(Stmt&& s) -> std::list<Stmt>
  {
    return linear(do_stmt(std::move(s)), {});
  }

  static auto basic_blocks(std::list<Stmt>& l)
    -> std::pair<std::vector<BasicBlock>, TempGen::Label>;
  static auto trace_schedule(std::vector<BasicBlock>& blocks, const TempGen::Label& ldone)
    -> std::list<Stmt>;

  // do_exp
  auto operator()(std::unique_ptr<ConstExp> e) -> std::pair<Stmt, Exp>;
  auto operator()(std::unique_ptr<NameExp> e) -> std::pair<Stmt, Exp>;
  auto operator()(std::unique_ptr<TempExp> e) -> std::pair<Stmt, Exp>;
  auto operator()(std::unique_ptr<BinOpExp> e) -> std::pair<Stmt, Exp>;
  auto operator()(std::unique_ptr<MemExp> e) -> std::pair<Stmt, Exp>;
  auto operator()(std::unique_ptr<CallExp> e) -> std::pair<Stmt, Exp>;
  auto operator()(std::unique_ptr<ESeqExp> e) -> std::pair<Stmt, Exp>;

  // do_stmt
  auto operator()(std::unique_ptr<ExpStmt> s) -> Stmt;
  auto operator()(std::unique_ptr<MoveStmt> s) -> Stmt;
  auto operator()(std::unique_ptr<JumpStmt> s) -> Stmt;
  auto operator()(std::unique_ptr<CJumpStmt> s) -> Stmt;
  auto operator()(std::unique_ptr<SeqStmt> s) -> Stmt;
  auto operator()(std::unique_ptr<LabelStmt> s) -> Stmt;

  private:
  auto do_exp(Exp&& e) -> std::pair<Stmt, Exp>;
  auto do_stmt(Stmt&& s) -> Stmt;
  auto reorder_exp(std::list<Exp>&& el, const std::function<Exp(std::list<Exp>&&)>& build_fn)
    -> std::pair<Stmt, Exp>;
  auto reorder_stmt(std::list<Exp>&& l, const std::function<Stmt(std::list<Exp>&&)>& build_fn)
    -> Stmt;
  auto reorder(std::list<Exp>&& el) -> std::pair<Stmt, std::list<Exp>>;
  static auto commute(const Stmt& stmt, const Exp& exp) -> bool;
  static auto concat(Stmt&& s1, Stmt&& s2) -> Stmt;
  static auto linear(Stmt&& s, std::list<Stmt>&& l) -> std::list<Stmt>;
};
} // namespace ir::tree