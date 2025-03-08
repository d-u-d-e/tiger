#pragma once
#include <memory>
#include <symbol.hpp>
#include <vector>

namespace semantic::types
{

class Type {
  public:
  virtual ~Type() = default;
  virtual std::string to_string() = 0;
};

struct Integer : public Type {
  std::string to_string() override;
};
struct String : public Type {
  std::string to_string() override;
};
struct Nil : public Type {
  std::string to_string() override;
};

// Used to indicate that an expression returns no value
struct Unit : public Type {
  std::string to_string() override;
};

struct Record : public Type {
  Record(std::vector<std::pair<symbol::Symbol, std::shared_ptr<Type>>> fields)
    : fields(std::move(fields))
  { }

  std::string to_string() override;
  std::vector<std::pair<symbol::Symbol, std::shared_ptr<Type>>> fields;
};

struct Array : public Type {
  Array(std::shared_ptr<Type> type)
    : type(std::move(type))
  { }

  std::string to_string() override;
  std::shared_ptr<Type> type;
};

struct Name : public Type {
  Name(const symbol::Symbol& name, std::shared_ptr<Type> type)
    : name(name)
    , type(std::move(type))
  { }

  std::string to_string() override;
  symbol::Symbol name;
  std::shared_ptr<Type> type;
};
} // namespace semantic::types
