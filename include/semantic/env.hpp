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

using VEntry = std::variant<VarEntry, FuncEntry>;
using TEntry = std::shared_ptr<types::Type>;

template <typename T>
class Environment {
  public:
  Environment() = default;
  void enter(const symbol::Identifier& id, const T& value) {
    table[id].push_front(value);
  }
  std::string dump() const {
    std::string result;
    for(const auto& [id, l] : table) {
      
      result += std::to_string(id) + ":\n";
      for (const auto& v : l) {
        result += std::format("   {}", v->to_string());
        result += "\n";
      }
      result += "\n"; 
    }
    return result;
  }

  private:
  std::unordered_map<symbol::Identifier, std::list<T>> table;
};

} // namespace environment

} // namespace semantic
