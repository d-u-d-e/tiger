#include <arch/frame.hpp>
#include <cassert>
#include <ir/translator.hpp>

namespace ir
{

std::unique_ptr<Exp> Translator::simple_var(const Level::Access& var_ax,
                                            const Level* current)
{
  std::unique_ptr<Exp> fp = std::make_unique<TempExp>(arch::Frame::FP);

  while(var_ax.l != current) {
    // first arg holds the static link
    auto slink = current->formals[0].fax;
    fp = arch::Frame::exp(slink, std::move(fp));
    current = current->parent;
    assert(current != nullptr);
  }
  return arch::Frame::exp(var_ax.fax, std::move(fp));
}

std::unique_ptr<ir::Exp> Translator::seq_exp(std::vector<std::unique_ptr<ir::Exp>>&& exps)
{
  auto size = exps.size();
  if(0 == size) {
    return std::make_unique<ir::ConstExp>(0);
  }
  else if(1 == size) {
    return std::move(exps[0]);
  }

  std::unique_ptr<ir::Stmt> stmt_seq =
    std::make_unique<ir::ExpStmt>(std::move(exps[0]));

  for(auto i = 1; i < size - 1; i++) {
    auto exp_stmt = std::make_unique<ir::ExpStmt>(std::move(exps[i]));
    stmt_seq =
      std::make_unique<ir::SeqStmt>(std::move(stmt_seq), std::move(exp_stmt));
  }

  return std::make_unique<ir::ESeqExp>(move(stmt_seq),
                                       std::move(exps[size - 1]));
}

} // namespace ir