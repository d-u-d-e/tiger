#include <ir/translator.hpp>

namespace ir
{

std::unique_ptr<Exp> Translator::simple_var(const Level::Access& ax,
                                            const Level& current)
{
  // we need to generate an expression that follows the static links until we reach the FP of ax.l
  // TODO
  return nullptr;
}

} // namespace ir