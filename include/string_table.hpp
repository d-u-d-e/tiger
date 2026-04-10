#pragma once

#include "symbol.hpp"
#include <string>
#include <unordered_map>

class StringTable
{
  public:
  static auto name(const Symbol& symbol) -> std::string
  {
    return symbol.str();
  }
  auto symbol(const std::string& name) -> const Symbol&;
  auto dump() const -> std::string;

  private:
  Symbol::Identifier identifier{1};
  std::unordered_map<std::string, Symbol> table;
};