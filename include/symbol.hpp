#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>

namespace symbol
{
class Symbol {
  public:
  Symbol(const std::string& name, uint32_t identifier)
    : str(name)
    , identifier(identifier){};

  const std::string& name() const
  {
    return str;
  }
  uint32_t id() const
  {
    return identifier;
  }
  bool operator==(const Symbol& other) const
  {
    // fast check
    return identifier == other.identifier;
  }

  private:
  std::string str;
  uint32_t identifier;
};

class SymbolTable {
  public:
  std::string name(const Symbol& symbol) const
  {
    return symbol.name();
  }
  const Symbol& symbol(const std::string& name);

  std::string dump() const;

  private:
  uint32_t identifier{0};
  std::unordered_map<std::string, Symbol> table;
};
} // namespace symbol