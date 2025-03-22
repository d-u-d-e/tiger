#pragma once
#include <algorithm>
#include <cassert>
#include <format>
#include <ir/level.hpp>
#include <list>
#include <memory>
#include <optional>
#include <seman/types.hpp>
#include <stack>
#include <variant>

namespace seman::env
{
using namespace types;
using namespace symbol;

class VarEntry {
  public:
  explicit VarEntry(shared_type_t type, ir::Level::Access access)
    : type(std::move(type))
    , access(std::move(access))
  { }

  shared_type_t type;
  ir::Level::Access access; // tells where the variable resides in memory
};

class FuncEntry {
  public:
  explicit FuncEntry(ir::Temp::label_t name,
                     std::vector<shared_type_t> formals,
                     shared_type_t result,
                     std::shared_ptr<ir::Level> level)
    : label(name)
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
  Environment()
  {
    table = std::make_unique<std::list<std::pair<Symbol, T>>[]>(capacity);
  }

  void begin_scope()
  {

    depth_++;
    extern Symbol scope_marker;
    // push a scope_marker
    stack.push(scope_marker);
  }

  void end_scope()
  {
    depth_--;
    extern Symbol scope_marker;
    // pop all elements until a scope_marker is found
    while(true) {
      assert(stack.size() > 0);
      auto elem = stack.top();
      stack.pop();
      if(elem == scope_marker) {
        break;
      }
      pop(elem);
    }
  }

  using const_iterator = const std::list<std::pair<Symbol, T>>*;

  template <typename U>
  void enter(const Symbol& s, U&& value)
  {
    stack.push(s);
    if(count + 1 > capacity * load_factor) {
      grow();
    }
    size_t index = s.id() % capacity;
    table[index].emplace_front(s, std::forward<U>(value));
    count++;
  }

  const T* lookup(const Symbol& s) const
  {
    size_t index = s.id() % capacity;
    auto iter = std::find_if(table[index].begin(),
                             table[index].end(),
                             [&s](const std::pair<Symbol, T>& pair) -> bool {
                               return std::get<0>(pair).id() == s.id();
                             });

    if(iter == table[index].end()) {
      return nullptr;
    }
    return &std::get<1>(*iter);
  }

  const_iterator begin() const
  {
    if(count == 0) {
      return end();
    }
    return &table[0];
  }

  const_iterator end() const
  {
    return &table[capacity];
  }

  size_t size() const
  {
    return count;
  }

  int depth() const
  {
    return depth_;
  }

  template <typename U>
  void replace(const Symbol& s, U&& value)
  {
    size_t index = s.id() % capacity;
    auto iter = std::find_if(table[index].begin(),
                             table[index].end(),
                             [&s](const std::pair<Symbol, T>& pair) -> bool {
                               return std::get<0>(pair).id() == s.id();
                             });
    assert(iter != table[index].end());
    (*iter).second = std::forward<U>(value);
  }

  template <
    typename U = T,
    bool has_to_string =
      std::is_same_v<decltype(std::declval<U>().to_string()), std::string>>
  std::string dump() const
  {
    std::string result;
    for(size_t i = 0; i < capacity; i++) {
      auto& l = table[i];
      if(l.size() == 0) {
        continue;
      }
      result += "-----------------\n";
      for(const auto& [s, v] : l) {
        result += std::to_string(s.id()) + "-> " + "\"" + s.str() +
                  "\": " + v.to_string() + "\n";
      }
    }
    return result;
  }

  private:
  void pop(const Symbol& s)
  {
    size_t index = s.id() % capacity;
    assert(table[index].size() > 0);
    std::pair<Symbol, T>& front = table[index].front();
    assert(std::get<0>(front) == s);
    table[index].pop_front();
    count--;
  }

  void grow()
  {
    auto capacity_old = capacity;
    capacity *= 2;
    auto new_table =
      std::make_unique<std::list<std::pair<Symbol, T>>[]>(capacity);

    for(size_t i = 0; i < capacity_old; i++) {
      auto& l = table[i];
      for(auto& [s, v] : l) {
        auto bin = s.id() % capacity;
        new_table[bin].emplace_back(s, std::move(v));
      }
    }
    table = std::move(new_table);
  }

  float load_factor{0.75};
  std::unique_ptr<std::list<std::pair<Symbol, T>>[]> table;
  size_t capacity{8};
  size_t count{0};
  int depth_{0};

  std::stack<symbol::Symbol> stack;
};

} // namespace seman::env