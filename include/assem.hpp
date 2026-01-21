#pragma once
#include "temp.hpp"
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace assem
{

struct Oper
{
  std::string assem;
  std::vector<TempGen::Temp> dst;
  std::vector<TempGen::Temp> src;
  std::optional<std::vector<TempGen::Label>> jmp;
};

struct Label
{
  std::string assem;
  TempGen::Label label;
};

struct Move
{
  std::string assem;
  TempGen::Temp dst;
  TempGen::Temp src;
  bool operator==(const Move& rhs) const
  {
    return dst == rhs.dst && src == rhs.src;
  }
};

using Instruction = std::variant<Oper, Label, Move>;

} // namespace assem
