#pragma once
#include <format>
#include <memory>
#include <optional>
#include <semantic/types.hpp>
#include <stack>
#include <translation/level.hpp>
#include <variant>

namespace semantic::env
{
using namespace types;

class VarEntry {
  public:
  explicit VarEntry(shared_type_t type, translation::Level::Access access)
    : type(std::move(type))
    , access(std::move(access))
  { }

  translation::Level::Access
    access; // tells where the variable resides in memory
  shared_type_t type;
};

class FuncEntry {
  public:
  explicit FuncEntry(std::vector<shared_type_t> formals,
                     shared_type_t result,
                     std::shared_ptr<translation::Level> level)
    : label(translation::Temp::new_label())
    , formals(std::move(formals))
    , result(std::move(result))
    , level(std::move(level))
  { }

  translation::Temp::label_t label;
  std::vector<shared_type_t> formals;
  shared_type_t result;
  std::shared_ptr<translation::Level> level{};
};

using VEntry = std::variant<std::monostate, VarEntry, FuncEntry>;

struct TEntry {
  shared_type_t t;
};

std::string to_string(const VEntry& entry);
std::string to_string(const TEntry& entry);

template <typename T>
class Environment {
  public:
  Environment() = default;

  template <typename U>
  void enter(const symbol::Symbol& s, U&& value)
  {
    table.enter(s, std::forward<U>(value));
    stack.push(s);
  };

  const T* lookup(const symbol::Symbol& s) const
  {
    return table.lookup(s);
  }

  template <typename U>
  void replace(const symbol::Symbol& s, U&& value)
  {
    table.replace(s, std::forward<U>(value));
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
        result += std::to_string(s.id()) + "-> " + "\"" + s.str() +
                  "\": " + to_string(v) + "\n";
      }
    }
    return result;
  }

  private:
  std::stack<symbol::Symbol> stack;
  symbol::Table<T> table;
};

} // namespace semantic::env
