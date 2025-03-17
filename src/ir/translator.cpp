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
    auto slink = current->formals_[0].fax;
    fp = arch::Frame::exp(slink, std::move(fp));
    current = current->parent;
    assert(current != nullptr);
  }
  return arch::Frame::exp(var_ax.fax, std::move(fp));
}

} // namespace ir