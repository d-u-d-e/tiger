#pragma once
#include "symbol.hpp"
#include <cassert>
#include <list>
#include <memory>
#include <stack>
#include <algorithm>

namespace semant
{

template <typename T>
class Environment
{
  public:
  static inline Symbol scope_marker{"", 0};
  using const_iterator = const std::list<std::pair<Symbol, T>>*;

  Environment()
  {
    table = std::make_unique<std::list<std::pair<Symbol, T>>[]>(capacity);
  }

  void begin_scope()
  {
    depth_++;
    // push the scope_marker
    stack.push(scope_marker);
  }

  void end_scope()
  {
    depth_--;
    // pop all elements until the scope_marker is found
    while(true)
    {
      assert(stack.size() > 0);
      auto elem = stack.top();
      stack.pop();
      if(elem == scope_marker)
      {
        break;
      }
      pop(elem);
    }
  }

  template <typename U>
  void enter(const Symbol& s, U&& value)
  {
    stack.push(s);
    if(count + 1 > capacity * load_factor)
    {
      grow();
    }
    size_t index = s.id() % capacity;
    table[index].emplace_front(s, std::forward<U>(value));
    count++;
  }

  auto lookup(const Symbol& s) const -> const T*
  {
    size_t index = s.id() % capacity;
    auto iter = std::find_if(
      table[index].begin(), table[index].end(), [&s](const std::pair<Symbol, T>& pair) -> bool {
        return std::get<0>(pair).id() == s.id();
      });

    if(iter == table[index].end())
    {
      return nullptr;
    }
    return &std::get<1>(*iter);
  }

  auto begin() const -> const_iterator
  {
    if(count == 0)
    {
      return end();
    }
    return &table[0];
  }

  auto end() const -> const_iterator
  {
    return &table[capacity];
  }

  [[nodiscard]] auto size() const -> size_t
  {
    return count;
  }

  [[nodiscard]] auto depth() const -> int
  {
    return depth_;
  }

  template <typename U>
  void replace(const Symbol& s, U&& value)
  {
    size_t index = s.id() % capacity;
    auto iter = std::find_if(
      table[index].begin(), table[index].end(), [&s](const std::pair<Symbol, T>& pair) -> bool {
        return std::get<0>(pair).id() == s.id();
      });
    assert(iter != table[index].end());
    (*iter).second = std::forward<U>(value);
  }

  [[nodiscard]] auto dump() const -> std::string
    requires requires(const T& t) {
      { t.to_string() } -> std::same_as<std::string>;
    }
  {
    std::string result;
    for(size_t i = 0; i < capacity; i++)
    {
      auto& l = table[i];
      if(l.size() == 0)
      {
        continue;
      }
      result += "-----------------\n";
      for(const auto& [s, v] : l)
      {
        result += std::to_string(s.id()) + "-> " + "\"" + s.str() + "\": " + v.to_string() + "\n";
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
    auto new_table = std::make_unique<std::list<std::pair<Symbol, T>>[]>(capacity);

    for(size_t i = 0; i < capacity_old; i++)
    {
      auto& l = table[i];
      for(auto& [s, v] : l)
      {
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

  std::stack<Symbol> stack;
};

} // namespace semant