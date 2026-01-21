#pragma once
#include "temp.hpp"
#include <functional>
#include <list>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace assem
{

using register_t = std::string;

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

std::string format(std::function<register_t(const TempGen::Temp& t)> mapper,
                   const Instruction& ins);

void delete_coalesced_moves(std::list<Instruction>& instrs,
                            const std::function<register_t(const TempGen::Temp&)>& reg_mapper);
} // namespace assem
