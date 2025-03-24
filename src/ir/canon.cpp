#include <ir/canon.hpp>

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
  // This mimics do_exp from Appel's solutions (so much verbose here, thanks C++)

  auto visitor = overloads{
    [&exp](std::unique_ptr<ConstExp> e) {
      // nothing to do
      return std::make_pair<Stmt, Exp>(
        std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)), std::move(e));
    },

    [&exp](std::unique_ptr<NameExp> e) {
      // nothing to do
      return std::make_pair<Stmt, Exp>(
        std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)), std::move(e));
    },

    [&exp](std::unique_ptr<TempExp> e) {
      // nothing to do
      return std::make_pair<Stmt, Exp>(
        std::make_unique<ExpStmt>(std::make_unique<ConstExp>(0)), std::move(e));
    },

    [&exp, this](std::unique_ptr<BinOpExp> e) {
      std::list<Exp> subexps;
      subexps.push_back(std::move(e->left));
      subexps.push_back(std::move(e->right));
      return reorder_exp(std::move(subexps), [op = e->op](std::list<Exp>&& l) {
        auto left = std::move(l.front());
        l.pop_front();
        auto right = std::move(l.front());
        l.pop_front();
        return std::make_unique<BinOpExp>(
          op, std::move(left), std::move(right));
      });
    },

    [&exp, this](std::unique_ptr<MemExp> e) {
      std::list<Exp> subexps;
      subexps.push_back(std::move(e->a));

      return reorder_exp(std::move(subexps), [](std::list<Exp>&& l) {
        auto a = std::move(l.front());
        l.pop_front();
        return std::make_unique<MemExp>(std::move(a));
      });
    },

    [&exp, this](std::unique_ptr<CallExp> e) {
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
    },

    [&exp, this](std::unique_ptr<ESeqExp> e) {
      Stmt s = do_stmt(std::move(e->stmt));
      auto [stmt, e_] = do_exp(std::move(e->exp));
      return std::make_pair(concat(std::move(s), std::move(stmt)),
                            std::move(e_));
    }};

  return std::visit(visitor, std::move(exp));
}

Stmt Canon::do_stmt(Stmt&& s)
{
  // This mimics do_exp from Appel's solutions (so much verbose here, thanks C++)

  auto visitor = overloads{[](std::unique_ptr<MoveStmt> s) {
                             // TODO
                             return Stmt(std::move(s));
                           },

                           [](std::unique_ptr<ExpStmt> s) {
                             // TODO
                             return Stmt(std::move(s));
                           },

                           [](std::unique_ptr<JumpStmt> s) {
                             // TODO
                             return Stmt(std::move(s));
                           },

                           [](std::unique_ptr<CJumpStmt> s) {
                             // TODO
                             return Stmt(std::move(s));
                           },

                           [](std::unique_ptr<SeqStmt> s) {
                             // TODO
                             return Stmt(std::move(s));
                           },

                           [](std::unique_ptr<LabelStmt> s) {
                             // TODO
                             return Stmt(std::move(s));
                           }};

  return std::visit(visitor, std::move(s));
}

} // namespace ir::tree