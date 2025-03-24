#pragma once

#include <arch/frame.hpp>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <memory>
#include <string>
#include <variant>

namespace ir
{
struct ProcedureFragment {
  std::unique_ptr<ir::Stmt> body;
  arch::Frame frame;
};

struct StringFragment {
  Temp::label_t label;
  std::string lit;
};

using Fragment = std::variant<StringFragment, ProcedureFragment>;

} // namespace ir