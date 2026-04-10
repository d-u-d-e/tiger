#pragma once
#include "ir/tree.hpp"
#include "symbol.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace semant::types
{

class Type
{
  public:
  Type() = default;
  virtual ~Type() = default;

  Type(const Type&) = delete;
  auto operator=(const Type&) -> Type& = delete;
  Type(Type&&) = delete;
  auto operator=(Type&&) -> Type& = delete;

  virtual auto to_string() -> std::string = 0;
};

using SharedType = std::shared_ptr<Type>;

struct Integer : public Type
{
  auto to_string() -> std::string override;
};

struct String : public Type
{
  auto to_string() -> std::string override;
};

struct Nil : public Type
{
  auto to_string() -> std::string override;
};

// Used to indicate that an expression returns no value
struct Unit : public Type
{
  auto to_string() -> std::string override;
};

struct Record : public Type
{
  explicit Record(std::vector<std::pair<Symbol, SharedType>> fields)
    : fields(std::move(fields))
  { }

  auto to_string() -> std::string override;
  std::vector<std::pair<Symbol, SharedType>> fields;
};

struct Array : public Type
{
  explicit Array(SharedType type)
    : type(std::move(type))
  { }

  auto to_string() -> std::string override;
  SharedType type;
};

struct Name : public Type
{
  explicit Name(Symbol name, SharedType type)
    : name(std::move(name))
    , type(std::move(type))
  { }

  auto to_string() -> std::string override;
  Symbol name;
  SharedType type;
};

// A function type is something like "(T1, ...) -> R" or "() -> R"
struct FunctionType : public Type
{

  explicit FunctionType(std::vector<SharedType> arg_types, SharedType result_type)
    : formals(std::move(arg_types))
    , ret(std::move(result_type))
  { }

  auto to_string() -> std::string override;
  std::vector<SharedType> formals;
  SharedType ret;
};

struct Result
{
  SharedType type;
  ir::Exp ir;
};

auto to_string(const SharedType& t) -> std::string;

} // namespace semant::types
