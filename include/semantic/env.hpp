#pragma once
#include <list>
#include <memory>
#include <semantic/types.hpp>
#include <symbol.hpp>
#include <unordered_map>
#include <variant>

namespace semantic
{

namespace environment
{

class VarEntry {
  public:
  VarEntry(std::shared_ptr<types::Type> type)
    : type(std::move(type))
  { }
  std::shared_ptr<types::Type> type;
};

class FuncEntry {
  public:
  FuncEntry(std::vector<std::shared_ptr<types::Type>> formals,
            std::shared_ptr<types::Type> result)
    : formals(std::move(formals))
    , result(std::move(result))
  { }
  std::vector<std::shared_ptr<types::Type>> formals;
  std::shared_ptr<types::Type> result;
};

using Entry = std::variant<VarEntry, FuncEntry>;

template <typename T>
class Environment {
  public:
  Environment() = default;

  private:
  std::unordered_map<symbol::Identifier, std::list<T>> table;
};

} // namespace environment

} // namespace semantic
