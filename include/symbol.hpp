#pragma once
#include <cstdint>
#include <string>
#include <utility>

class Symbol
{
  public:
  using Identifier = uint32_t;
  Symbol(std::string name, Identifier id)
    : name(std::move(name))
    , id_(id){};

  [[nodiscard]] auto str() const -> const std::string&
  {
    return name;
  }

  [[nodiscard]] auto id() const -> Identifier
  {
    return id_;
  }

  auto operator==(const Symbol& other) const -> bool
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
  auto operator()(const Symbol& s) const -> size_t
  {
    return s.id();
  }
};