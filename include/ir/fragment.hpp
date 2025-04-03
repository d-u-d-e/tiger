#pragma once

#include <codegen/arch/frame.hpp>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <string>
#include <variant>

namespace ir
{
struct ProcedureFragment {
  Nx body;
  arch::Frame frame;
};

struct StringFragment {
  TempGen::Label label;
  std::string lit;
};

using Fragment = std::variant<StringFragment, ProcedureFragment>;

} // namespace ir