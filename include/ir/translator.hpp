#pragma once
#include <arch/frame.hpp>
#include <ir/level.hpp>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <memory>

namespace ir
{

class Translator {
  public:
  Translator()
  {
    lvl_outermost = std::make_shared<Level>(
      nullptr, arch::Frame(Temp::named_label("outermost"), {}));
  }

  std::shared_ptr<Level> outermost_level()
  {
    return lvl_outermost;
  }

  static std::unique_ptr<Level> new_level(const Level* parent,
                                          Temp::label_t label,
                                          const std::vector<bool>& formals)
  {
    // augment the formals with the static link as first parameter
    std::vector<bool> with_slink(formals.size() + 1);
    std::copy(formals.begin(), formals.end(), with_slink.begin() + 1);
    with_slink[0] = true;
    return std::make_unique<Level>(parent, arch::Frame(label, with_slink));
  }

  static const std::vector<Level::Access>& formals(const Level& level)
  {
    return level.formals;
  }

  static Level::Access alloc_local(Level& level, bool escape)
  {
    return Level::Access{.l = &level, .fax = level.f.alloc_local(escape)};
  }

  std::unique_ptr<Exp> simple_var(const Level::Access& ax,
                                  const Level* current);
  std::unique_ptr<ir::Exp> seq_exp(std::vector<std::unique_ptr<ir::Exp>>&& exps);
  std::unique_ptr<ir::Exp> constant(int constant);

  private:
  std::shared_ptr<Level> lvl_outermost;
};

} // namespace ir