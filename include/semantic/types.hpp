#pragma once
#include <memory>
#include <optional>
#include <symbol.hpp>
#include <vector>

namespace semantic
{

namespace types
{

class Type {
  public:
  virtual ~Type() = default;
};

struct Integer : public Type { };
struct String : public Type { };
struct Nil : public Type { };
struct Unit : public Type {
}; // Used to indicate that an expression returns no value

struct Record : public Type {
  Record(std::vector<std::pair<symbol::Symbol, std::shared_ptr<Type>>> fields,
         uint32_t unique)
    : fields(std::move(fields))
    , unique(unique)
  { }

  bool operator==(const Record& other) const
  {
    return unique == other.unique;
  }

  std::vector<std::pair<symbol::Symbol, std::shared_ptr<Type>>> fields;
  uint32_t unique;
};

struct Array : public Type {
  Array(std::shared_ptr<Type> type, uint32_t unique)
    : type(std::move(type))
    , unique(unique)
  { }

  bool operator==(const Array& other) const
  {
    return unique == other.unique;
  }

  std::shared_ptr<Type> type;
  uint32_t unique;
};

struct Name : public Type {
  Name(const symbol::Symbol & name, std::shared_ptr<Type> type)
    : name(name)
    , type(std::move(type))
  { }

  symbol::Symbol name;
  std::shared_ptr<Type> type;
};

} // namespace types

}; // namespace semantic