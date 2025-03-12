#pragma once
#include <arch/frame.hpp>
#include <memory>
#include <translation/level.hpp>
#include <translation/temp.hpp>

namespace translation::ir
{

class Translator {
  public:

  Translator()
    : lvl_outermost(
        nullptr,
        arch::Frame(
          translation::Temp::getInstance().named_label("outermost"), {}))
  { }

  Level& outermost_level()
  {
    return lvl_outermost;
  }

  static Level new_level(const Level& parent,
                  Temp::label_t label,
                  const std::vector<bool>& formals)
  {
    return Level(&parent, arch::Frame(label, formals));
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
  Level lvl_outermost;
};

} // namespace translation::ir