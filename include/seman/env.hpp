#pragma once
#include <format>
#include <ir/level.hpp>
#include <memory>
#include <optional>
#include <seman/types.hpp>
#include <stack>
#include <variant>

namespace seman::env
{
using namespace types;

class VarEntry {
  public:
  explicit VarEntry(shared_type_t type, ir::Level::Access access)
    : type(std::move(type))
    , access(std::move(access))
  { }

  ir::Level::Access access; // tells where the variable resides in memory
  shared_type_t type;
};

class FuncEntry {
  public:
  explicit FuncEntry(std::vector<shared_type_t> formals,
                     shared_type_t result,
                     std::shared_ptr<ir::Level> level)
    : label(ir::Temp::new_label())
    , formals(std::move(formals))
    , result(std::move(result))
    , level(std::move(level))
  { }

  ir::Temp::label_t label;
  std::vector<shared_type_t> formals;
  shared_type_t result;
  std::shared_ptr<ir::Level> level{};
};

struct VEntry {
  std::string to_string() const;
  std::variant<std::monostate, VarEntry, FuncEntry> v;
};

struct TEntry {
  std::string to_string() const;
  shared_type_t t;
};

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
    return table.dump();
  }

  private:
  std::stack<symbol::Symbol> stack;
  symbol::Table<T> table;
};

} // namespace seman::env