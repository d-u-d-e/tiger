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
using shared_type_t = std::shared_ptr<Type>;

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
  explicit Record(std::vector<std::pair<symbol::Symbol, shared_type_t>> fields)
    : fields(std::move(fields))
  { }

  std::string to_string() override;
  std::vector<std::pair<symbol::Symbol, shared_type_t>> fields;
};

struct Array : public Type {
  explicit Array(shared_type_t type)
    : type(std::move(type))
  { }

  std::string to_string() override;
  shared_type_t type;
};

struct Name : public Type {
  explicit Name(const symbol::Symbol& name, shared_type_t type)
    : name(name)
    , type(std::move(type))
  { }

  std::string to_string() override;
  symbol::Symbol name;
  shared_type_t type;
};

struct Result {
  shared_type_t type;
  ir::exp_t ir;
};

std::string to_string(const shared_type_t& t);

} // namespace seman::types
