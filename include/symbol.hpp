#pragma once
#include <cstdint>
#include <string>

class Symbol
{
  public:
  using Identifier = uint32_t;
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

template <>
struct std::hash<Symbol>
{
  size_t operator()(const Symbol& s) const
  {
    return s.id();
  }
};