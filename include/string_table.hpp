#pragma once

#include "symbol.hpp"
#include <string>
#include <unordered_map>

class StringTable
{
  public:
  std::string name(const Symbol& symbol) const
  {
    return symbol.str();
  }
  const Symbol& symbol(const std::string& name);
  std::string dump() const;

  private:
  Symbol::Identifier identifier{1};
  std::unordered_map<std::string, Symbol> table;
};