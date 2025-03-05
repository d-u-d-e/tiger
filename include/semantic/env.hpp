#pragma once
#include <format>
#include <memory>
#include <optional>
#include <semantic/types.hpp>
#include <stack>
#include <variant>

namespace semantic::env
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

using VEntry = std::variant<std::monostate, VarEntry, FuncEntry>;
using TEntry = std::shared_ptr<types::Type>;
std::string to_string(const VEntry& entry);
std::string to_string(const TEntry& entry);

template <typename T>
class Environment {
  public:
  Environment() = default;

  void enter(const symbol::Symbol& s, const T& value)
  {
    table.enter(s, value);
    stack.push(s);
  };

  std::optional<T> lookup(const symbol::Symbol& s) const
  {
    return table.lookup(s);
  }

  void begin_scope()
  {
    // push a scope_marker
    stack.push(symbol::scope_marker);
  }

  void end_scope()
  {
    // pop all elements until a scope_marker is found
    while(true) {
      assert(stack.size() > 0);
      auto& elem = stack.top();
      stack.pop();
      if(elem == symbol::scope_marker) {
        break;
      }
      table.pop(elem);
    }
  }

  size_t size() const
  {
    return table.size();
  }

  std::string dump() const
  {
    std::string result;
    for(const auto& l : table) {
      if(l.size() == 0) {
        continue;
      }
      result += "-----------------\n";
      for(const auto& [s, v] : l) {
        result += std::to_string(s.id()) + "-> " + s.name() + ": " +
                  to_string(v) + "\n";
      }
    }
    return result;
  }

  private:
  std::stack<symbol::Symbol> stack;
  symbol::Table<T> table;
};

} // namespace semantic::env
