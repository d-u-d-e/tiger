#pragma once
#include <list>
#include <memory>
#include <semantic/types.hpp>
#include <stack>
#include <symbol.hpp>
#include <unordered_map>
#include <variant>
#include <optional>

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

  void enter(const symbol::Symbol& s, const T& value)
  {
    table[s].push_front(value);
    stack.push(s);
  }

  std::optional<T> lookup(const symbol::Symbol& s)
  {
    try {
      return table.at(s).front();
    }
    catch(std::out_of_range&) {
      return std::nullopt;
    }
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
      table.at(elem).pop_front();
    }
  }

  std::string dump() const
  {
    std::string result;
    for(const auto& [s, l] : table) {
      result += std::to_string(s.id()) + "-> " + s.name() + "\n";
      for(const auto& v : l) {
        result += std::format("   {}\n", v->to_string());
      }
      result += "--------------------\n";
    }
    return result;
  }

  struct KeyHasher {
    size_t operator()(const symbol::Symbol& a) const
    {
      return a.id();
    }
  };

  private:
  std::stack<symbol::Symbol> stack;
  // This is an overkill, we only need a table with chaining
  std::unordered_map<symbol::Symbol, std::list<T>, KeyHasher> table;
};

} // namespace environment

} // namespace semantic
