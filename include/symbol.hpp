#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>

namespace symbol
{

using Identifier = uint32_t;

class Symbol {
  public:
  Symbol(const std::string& name, Identifier id)
    : name(name)
    , id_(id){};

  const std::string& str() const
  {
    return name;
  }
  Identifier id() const
  {
    return id_;
  }
  bool operator==(const Symbol& other) const
  {
    // fast check
    return id_ == other.id_;
  }

  private:
  std::string name;
  Identifier id_;
};

class StringTable {
  public:
  std::string name(const Symbol& symbol) const
  {
    return symbol.str();
  }
  const Symbol& symbol(const std::string& name);
  std::string dump() const;

  private:
  Identifier identifier{1};
  std::unordered_map<std::string, Symbol> table;
};

} // namespace symbol