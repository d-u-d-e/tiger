#include <arch/frame.hpp>
#include <cassert>
#include <ir/translator.hpp>

namespace ir
{

ir::exp_t Translator::simple_var(const Level::Access& var_ax,
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

ir::exp_t Translator::seq_exp(std::vector<ir::exp_t>&& exps)
{
  auto size = exps.size();
  if(0 == size) {
    return std::make_unique<ir::ConstExp>(0);
  }
  else if(1 == size) {
    return std::move(exps[0]);
  }

  auto stmt_seq = unnx(std::move(exps[0]));

  for(auto i = 1; i < size - 1; i++) {
    auto exp_stmt = unnx(std::move(exps[i]));
    stmt_seq =
      std::make_unique<ir::SeqStmt>(std::move(stmt_seq), std::move(exp_stmt));
  }

  return std::make_unique<ir::ESeqExp>(move(stmt_seq),
                                       unex(std::move(exps[size - 1])));
}

ex_t Translator::constant(int constant)
{
  return std::make_unique<ir::ConstExp>(constant);
}

exp_t Translator::call_exp(Temp::label_t flab, std::vector<ir::ex_t>&& args)
{
  return std::make_unique<ir::CallExp>(std::make_unique<NameExp>(flab),
                                       std::move(args));
}

ex_t Translator::unex(exp_t&& exp)
{
  if(std::holds_alternative<ex_t>(exp)) {
    return std::move(std::get<ex_t>(exp));
  }
  else if(std::holds_alternative<nx_t>(exp)) {
    return std::make_unique<ir::ESeqExp>(std::move(std::get<nx_t>(exp)),
                                         constant(0));
  }
  else if(std::holds_alternative<cx_t>(exp)) {
    // TODO
    // 
    assert(false);
  }
  return nullptr;
}

nx_t Translator::unnx(exp_t&& exp)
{
  if(std::holds_alternative<ex_t>(exp)) {
    return std::make_unique<ir::ExpStmt>(std::move(std::get<ex_t>(exp)));
  }
  else if(std::holds_alternative<nx_t>(exp)) {
    return std::move(std::get<nx_t>(exp));
  }
  else if(std::holds_alternative<cx_t>(exp)) {
    // TODO
    assert(false);
  }
  return nullptr;
}

cx_t Translator::uncx(exp_t&& exp)
{
  // TODO
  return [](Temp::label_t a, Temp::label_t b) { return nullptr; };
}

} // namespace ir