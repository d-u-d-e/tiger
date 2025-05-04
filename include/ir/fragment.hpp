#pragma once

#include <ir/temp.hpp>
#include <ir/level.hpp>
#include <ir/tree.hpp>
#include <memory>
#include <string>
#include <variant>

namespace ir
{
struct ProcedureFragment {
  Nx body;
  std::shared_ptr<Level> level;
};

struct StringFragment {
  TempGen::Label label;
  std::string lit;
};

using Fragment = std::variant<StringFragment, ProcedureFragment>;

} // namespace ir