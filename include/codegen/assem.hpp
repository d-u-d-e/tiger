#pragma once
#include <ir/temp.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace codegen
{
namespace assem
{

struct Oper {
  std::string assem;
  std::vector<ir::TempGen::Temp> dst;
  std::vector<ir::TempGen::Temp> src;
  std::optional<std::vector<ir::TempGen::Label>> jmp;
};

struct Label {
  std::string assem;
  ir::TempGen::Label label;
};

struct Move {
  std::string assem;
  ir::TempGen::Temp dst;
  ir::TempGen::Temp src;
};

using Instruction = std::variant<Oper, Label, Move>;

} // namespace assem
} // namespace codegen