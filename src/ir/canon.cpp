#include "ir/canon.hpp"
#include "ir/tree.hpp"
#include "temp.hpp"
#include <cassert>
#include <memory>

namespace ir::tree
{

std::pair<Stmt, Exp> Canon::reorder_exp(std::list<Exp>&& l,
                                        std::function<Exp(std::list<Exp>&&)> build_fn)
{
  auto [stmt, el] = reorder(std::move(l));
  return std::make_pair(std::move(stmt), build_fn(std::move(el)));
}

Stmt Canon::reorder_stmt(std::list<Exp>&& l, std::function<Stmt(std::list<Exp>&&)> build_fn)
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
  return std::make_pair<Stmt, Exp>(std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)),
                                   std::move(e));
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<NameExp> e)
{
  // nothing to do
  return std::make_pair<Stmt, Exp>(std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)),
                                   std::move(e));
}

std::pair<Stmt, Exp> Canon::operator()(std::unique_ptr<TempExp> e)
{
  // nothing to do
  return std::make_pair<Stmt, Exp>(std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)),
                                   std::move(e));
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
  std::move(e->args.begin(), e->args.end(), std::back_inserter(subexps));

  auto [stmt, ee] = reorder_exp(std::move(subexps), [](std::list<Exp>&& l) {
    auto f = std::move(l.front());
    l.pop_front();
    std::vector<Exp> args;
    std::move(l.begin(), l.end(), std::back_inserter(args));
    return std::make_unique<CallExp>(std::move(f), std::move(args));
  });

  auto t = TempGen::new_temp();
  return {concat(std::move(stmt),
                 std::make_unique<MoveStmt>(std::make_unique<TempExp>(t), std::move(ee))),
          std::make_unique<TempExp>(t)};
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
  if(std::holds_alternative<std::unique_ptr<CallExp>>(s->exp))
  {
    // just a call expression which discards the return value
    auto ce = std::move(std::get<std::unique_ptr<CallExp>>(s->exp));
    subexps.push_back(std::move(ce->fun));
    std::move(ce->args.begin(), ce->args.end(), std::back_inserter(subexps));

    return reorder_stmt(std::move(subexps), [](std::list<Exp>&& l) {
      auto f = std::move(l.front());
      l.pop_front();
      std::vector<Exp> args;
      std::move(l.begin(), l.end(), std::back_inserter(args));
      return std::make_unique<ExpStmt>(std::make_unique<CallExp>(std::move(f), std::move(args)));
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
  return reorder_stmt(std::move(subexps), [labs = s->labels](std::list<Exp>&& l) {
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
    std::move(subexps), [tlab = s->tlabel, flab = s->flabel, op = s->op](std::list<Exp>&& l) {
      auto lexp = std::move(l.front());
      l.pop_front();
      auto rexp = std::move(l.front());
      l.pop_front();
      return std::make_unique<CJumpStmt>(op, std::move(lexp), std::move(rexp), tlab, flab);
    });
}

Stmt Canon::operator()(std::unique_ptr<SeqStmt> s)
{
  return concat(do_stmt(std::move(s->stm1)), do_stmt(std::move(s->stm2)));
}

Stmt Canon::operator()(std::unique_ptr<LabelStmt> s)
{
  return Stmt(std::move(s));
}

Stmt Canon::operator()(std::unique_ptr<MoveStmt> s)
{
  if(std::holds_alternative<std::unique_ptr<TempExp>>(s->left))
  {
    // moving to a temporary
    auto temp = std::get<std::unique_ptr<TempExp>>(s->left)->temp;
    std::list<Exp> subexps;

    if(std::holds_alternative<std::unique_ptr<CallExp>>(s->right))
    {
      // here we need to make sure to not call reorder on the CallExp
      auto& e = std::get<std::unique_ptr<CallExp>>(s->right);
      subexps.push_back(std::move(e->fun));
      std::move(e->args.begin(), e->args.end(), std::back_inserter(subexps));

      return reorder_stmt(std::move(subexps), [temp](std::list<Exp>&& l) {
        auto f = std::move(l.front());
        l.pop_front();
        std::vector<Exp> args;
        std::move(l.begin(), l.end(), std::back_inserter(args));
        return std::make_unique<MoveStmt>(std::make_unique<TempExp>(temp),
                                          std::make_unique<CallExp>(std::move(f), std::move(args)));
      });
    }
    else
    {
      subexps.push_back(std::move(s->right));
      return reorder_stmt(std::move(subexps), [temp](std::list<Exp>&& l) {
        auto right = std::move(l.front());
        l.pop_front();
        return std::make_unique<MoveStmt>(std::make_unique<TempExp>(temp), std::move(right));
      });
    }
  }
  else if(std::holds_alternative<std::unique_ptr<MemExp>>(s->left))
  {
    // moving to a memory location
    auto& mem_exp = std::get<std::unique_ptr<MemExp>>(s->left);
    std::list<Exp> subexps;
    subexps.push_back(std::move(mem_exp->a));
    subexps.push_back(std::move(s->right));
    return reorder_stmt(std::move(subexps), [](std::list<Exp>&& l) {
      auto a = std::move(l.front());
      l.pop_front();
      auto right = std::move(l.front());
      l.pop_front();
      return std::make_unique<MoveStmt>(std::make_unique<MemExp>(std::move(a)), std::move(right));
    });
  }
  // moving to an eseq expression
  assert(std::holds_alternative<std::unique_ptr<ESeqExp>>(s->left));
  auto& eseq = std::get<std::unique_ptr<ESeqExp>>(s->left);
  return do_stmt(std::make_unique<SeqStmt>(
    std::move(eseq->stmt), std::make_unique<MoveStmt>(std::move(eseq->exp), std::move(s->right))));
}

std::pair<Stmt, std::list<Exp>> Canon::reorder(std::list<Exp>&& el)
{
  // base
  if(el.empty())
  {
    return std::make_pair(std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)),
                          std::list<Exp>{});
  }

  auto front = std::move(el.front());
  el.pop_front();

  if(std::holds_alternative<std::unique_ptr<CallExp>>(front))
  {
    // all call expressions should put the result in a temporary
    auto t = TempGen::new_temp();
    el.push_front(std::make_unique<ESeqExp>(
      std::make_unique<MoveStmt>(std::make_unique<TempExp>(t), std::move(front)),
      std::make_unique<TempExp>(t)));

    return reorder(std::move(el));
  }

  auto [stmt, e] = do_exp(std::move(front));

  // reorder the rest
  auto [stmt_, el_] = reorder(std::move(el));

  // if stmt_ can commute, we can avoid moving to a temporary
  if(commute(stmt_, e))
  {
    el_.push_front(std::move(e));
    return {concat(std::move(stmt), std::move(stmt_)), std::move(el_)};
  }
  else
  {
    auto temp = TempGen::new_temp();
    auto a = concat(std::move(stmt),
                    std::make_unique<MoveStmt>(std::make_unique<TempExp>(temp), std::move(e)));

    el_.push_front(std::make_unique<TempExp>(temp));
    return {concat(std::move(a), std::move(stmt_)), std::move(el_)};
  }
}

std::pair<std::vector<Canon::BasicBlock>, TempGen::Label> Canon::basic_blocks(std::list<Stmt>&& l)
{
  /*
  From a list of cleaned trees, produce a list of
  basic blocks satisfying the following properties:
    1.  No SEQ's or ESEQ's
    2.  The parent of every CALL is an EXP(..) or a MOVE(TEMP t,..)
    3.  Every block begins with a LABEL;
    4.  A LABEL appears only at the beginning of a block;
    5.  Any JUMP or CJUMP is the last stm in a block;
    6.  Every block ends with a JUMP or CJUMP;
    Also produce the "label" to which control will be passed
    upon exit.
  */

  assert(l.size() > 0);
  std::vector<BasicBlock> blocks;
  auto ldone = TempGen::new_label();
  auto i = l.begin();

  while(i != l.end())
  {
    // start a new block
    BasicBlock b;

    // the new block must start with a label, create it if necessary
    if(!std::holds_alternative<std::unique_ptr<LabelStmt>>(*i))
    {
      b.stmts.emplace_back(std::make_unique<LabelStmt>(TempGen::new_label()));
    }
    else
    {
      b.stmts.emplace_back(std::move(*i));
      ++i;
    }

    // keep adding statements to the block until a new label is found or a jump/cjump is encountered
    while(true)
    {
      if(i == l.end())
      {
        // no jump at the end
        b.stmts.push_back(
          std::make_unique<JumpStmt>(std::make_unique<NameExp>(ldone), std::vector{ldone}));
        blocks.push_back(std::move(b));
        break;
      }
      else if(std::holds_alternative<std::unique_ptr<JumpStmt>>(*i) ||
              std::holds_alternative<std::unique_ptr<CJumpStmt>>(*i))
      {
        // found jump, end block and advance iterator
        b.stmts.emplace_back(std::move(*i));
        blocks.push_back(std::move(b));
        i++;
        break;
      }
      else if(std::holds_alternative<std::unique_ptr<LabelStmt>>(*i))
      {
        // end block by adding missing jump statement to current label statement
        auto lnext = std::get<std::unique_ptr<LabelStmt>>(*i)->label;
        b.stmts.push_back(
          std::make_unique<JumpStmt>(std::make_unique<NameExp>(lnext), std::vector{lnext}));
        blocks.push_back(std::move(b));
        break;
      }
      b.stmts.emplace_back(std::move(*i));
      ++i;
    }
  };

  return std::make_pair(std::move(blocks), ldone);
}

std::list<Stmt> Canon::trace_schedule(std::vector<BasicBlock>&& blocks, const TempGen::Label& ldone)
{
  /*
  From a list of basic blocks satisfying properties 1-6 above, 
  along with an "exit" label, produce a list of stms such that:

    1. and 2. as above;
    7. Every CJUMP(_,t,f) is immediately followed by LABEL f.
    The blocks are reordered to satisfy property 7; also
    in this reordering as many JUMP(T.NAME(lab)) statements
    as possible are eliminated by falling through into T.LABEL(lab).
  */

  std::list<Stmt> schedule;
  std::unordered_map<TempGen::Label, BasicBlock*> map;

  // add all blocks to the map
  for(auto& b : blocks)
  {
    auto& front = b.stmts.front();
    assert(std::holds_alternative<std::unique_ptr<LabelStmt>>(front));
    auto& lfront = std::get<std::unique_ptr<LabelStmt>>(front);
    map.emplace(lfront->label, &b);
  }

  for(auto& b : blocks)
  {
    if(b.visited)
    {
      continue;
    }
    // start a trace at b
    BasicBlock* bp = &b;

    while(bp)
    {
      BasicBlock* next{nullptr};
      bp->visited = true;
      auto& back = bp->stmts.back();
      auto& lfront = std::get<std::unique_ptr<LabelStmt>>(bp->stmts.front())->label;
      map.erase(lfront);

      if(std::holds_alternative<std::unique_ptr<CJumpStmt>>(back))
      {
        auto& cjump = std::get<std::unique_ptr<CJumpStmt>>(back);
        auto tb_iter = map.find(cjump->tlabel);
        auto fb_iter = map.find(cjump->flabel);

        if(fb_iter != map.end())
        {
          // the false block does not belong to any trace, follow it
          next = fb_iter->second;
        }
        else if(tb_iter != map.end())
        {
          // the true block does not belong to any trace
          // replace the current cjump with a cjump to the false label by inverting the rel op
          auto cjump_ = std::make_unique<CJumpStmt>(not_relop(cjump->op),
                                                    std::move(cjump->lexp),
                                                    std::move(cjump->rexp),
                                                    cjump->flabel,
                                                    cjump->tlabel);
          bp->stmts.pop_back(); // pop last cjump
          bp->stmts.push_back(std::move(cjump_));
          next = tb_iter->second;
        }
        else
        {
          // the true/false blocks belong to some trace
          // create a false label, and a new jump statement
          auto lfalse = TempGen::new_label();
          auto cjump_ = std::make_unique<CJumpStmt>(
            cjump->op, std::move(cjump->lexp), std::move(cjump->rexp), cjump->tlabel, lfalse);
          auto lstmt = std::make_unique<LabelStmt>(lfalse);
          auto jump = std::make_unique<JumpStmt>(std::make_unique<NameExp>(cjump->flabel),
                                                 std::vector{cjump->flabel});
          bp->stmts.pop_back(); // pop current cjump
          bp->stmts.push_back(std::move(cjump_));
          bp->stmts.push_back(std::move(lstmt));
          bp->stmts.push_back(std::move(jump));
          // cannot continue
        }
      }
      else
      {
        assert(std::holds_alternative<std::unique_ptr<JumpStmt>>(back));
        auto& jump = std::get<std::unique_ptr<JumpStmt>>(back);

        if(std::holds_alternative<std::unique_ptr<NameExp>>(jump->a))
        {
          // jump to a label
          auto ljump = std::get<std::unique_ptr<NameExp>>(jump->a)->label;
          auto iter = map.find(ljump);
          if(iter != map.end())
          {
            // throw away the jump statement, as we put the jump label next to it
            bp->stmts.pop_back();
            next = iter->second;
          }
        }
      }
      // add b to the trace
      std::move(bp->stmts.begin(), bp->stmts.end(), std::back_inserter(schedule));
      bp = next;
    }
  }

  // throw away useless final jmp
  if(std::holds_alternative<std::unique_ptr<ir::tree::JumpStmt>>(schedule.back()))
  {
    if(std::get<std::unique_ptr<ir::tree::JumpStmt>>(schedule.back())->labels == std::vector{ldone})
    {
      schedule.pop_back();
    }
  }
  // ldone is where the epilogue starts
  schedule.push_back(std::make_unique<LabelStmt>(ldone));
  return schedule;
}

std::list<Stmt> Canon::linear(Stmt&& s, std::list<Stmt>&& l)
{
  if(std::holds_alternative<std::unique_ptr<SeqStmt>>(s))
  {
    auto& seq = std::get<std::unique_ptr<SeqStmt>>(s);
    return linear(std::move(seq->stm1), linear(std::move(seq->stm2), std::move(l)));
  }
  else
  {
    l.push_front(std::move(s));
    return l;
  }
}

Stmt Canon::concat(Stmt&& s1, Stmt&& s2)
{
  auto throw_stmt = [](const Stmt& s) {
    if(std::holds_alternative<std::unique_ptr<ExpStmt>>(s))
    {
      if(std::holds_alternative<std::unique_ptr<ConstExp>>(
           std::get<std::unique_ptr<ExpStmt>>(s)->exp))
      {
        // s is useless
        return true;
      }
    }
    return false;
  };

  if(throw_stmt(s1))
  {
    return std::move(s2);
  }
  else if(throw_stmt(s2))
  {
    return std::move(s1);
  }
  return std::make_unique<SeqStmt>(std::move(s1), std::move(s2));
}

bool Canon::commute(const Stmt& stmt, const Exp& exp)
{
  if(std::holds_alternative<std::unique_ptr<ExpStmt>>(stmt))
  {
    auto& exp_stmt = std::get<std::unique_ptr<ExpStmt>>(stmt);
    if(std::holds_alternative<std::unique_ptr<ConstExp>>(exp_stmt->exp))
    {
      // an expression statement containing a constant commute with any expression
      return true;
    }
  }
  else if(std::holds_alternative<std::unique_ptr<NameExp>>(exp))
  {
    // a name expression commutes with any statement
    return true;
  }
  else if(std::holds_alternative<std::unique_ptr<ConstExp>>(exp))
  {
    // a constant expression commutes with any statement
    return true;
  }
  return false;
}

} // namespace ir::tree