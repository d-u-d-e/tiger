#pragma once
#include <arch/frame.hpp>
#include <ir/level.hpp>
#include <ir/temp.hpp>
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

  static std::unique_ptr<Level> new_level(const Level& parent,
                                          Temp::label_t label,
                                          const std::vector<bool>& formals)
  {
    return std::make_unique<Level>(&parent, arch::Frame(label, formals));
  }

  static const std::vector<Level::Access>& formals(const Level& level)
  {
    return level.formals_;
  }

  static Level::Access alloc_local(Level& level, bool escape)
  {
    return Level::Access{.l = &level, .access = level.f.alloc_local(escape)};
  }

  private:
  std::shared_ptr<Level> lvl_outermost;
};

} // namespace ir