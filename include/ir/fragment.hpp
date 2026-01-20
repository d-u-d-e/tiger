#pragma once

#include "ir/tree.hpp"
#include "level.hpp"
#include "temp.hpp"
#include <memory>
#include <string>
#include <variant>

namespace ir
{
template <typename FrameT>
struct ProcedureFragment
{
  Nx body;
  std::shared_ptr<Level<FrameT>> level;
};

struct StringFragment
{
  TempGen::Temp label;
  std::string lit;
};

template <typename FrameT>
using Fragment = std::variant<StringFragment, ProcedureFragment<FrameT>>;

} // namespace ir