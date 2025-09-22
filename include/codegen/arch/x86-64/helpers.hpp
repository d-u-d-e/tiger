#pragma once
#include <cassert>
#include <ir/fragment.hpp>
#include <string>

namespace arch
{
inline std::string emit_string(const ir::StringFragment& f)
{
  return std::format("{}:\n"
                     ".asciz \"{}\"\n",
                     f.label.str(),
                     f.lit);
}
} // namespace arch