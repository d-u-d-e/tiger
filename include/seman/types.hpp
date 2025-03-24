#pragma once
#include <ir/tree.hpp>
#include <memory>
#include <string>
#include <symbol.hpp>
#include <utility>
#include <vector>

namespace seman::types
{

class Type {
  public:
  virtual ~Type() = default;
  virtual std::string to_string() = 0;
};
using SharedType = std::shared_ptr<Type>;

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
  explicit Record(std::vector<std::pair<symbol::Symbol, SharedType>> fields)
    : fields(std::move(fields))
  { }

  std::string to_string() override;
  std::vector<std::pair<symbol::Symbol, SharedType>> fields;
};

struct Array : public Type {
  explicit Array(SharedType type)
    : type(std::move(type))
  { }

  std::string to_string() override;
  SharedType type;
};

struct Name : public Type {
  explicit Name(const symbol::Symbol& name, SharedType type)
    : name(name)
    , type(std::move(type))
  { }

  std::string to_string() override;
  symbol::Symbol name;
  SharedType type;
};

struct Result {
  SharedType type;
  ir::Exp ir;
};

std::string to_string(const SharedType& t);

} // namespace seman::types
