#pragma once
#include <algorithm>
#include <cassert>
#include <list>
#include <memory>
#include <optional>
#include <stdint.h>
#include <string>
#include <unordered_map>

namespace symbol
{

using Identifier = uint32_t;

class Symbol {
  public:
  Symbol(const std::string& name, Identifier id)
    : name(name)
    , id_(id){};

  const std::string& str() const
  {
    return name;
  }
  Identifier id() const
  {
    return id_;
  }
  bool operator==(const Symbol& other) const
  {
    // fast check
    return id_ == other.id_;
  }

  private:
  std::string name;
  Identifier id_;
};

class StringTable {
  public:
  std::string name(const Symbol& symbol) const
  {
    return symbol.str();
  }
  const Symbol& symbol(const std::string& name);
  std::string dump() const;

  private:
  Identifier identifier{1};
  std::unordered_map<std::string, Symbol> table;
};

extern Symbol scope_marker;

template <typename T>
class Table {
  public:
  using const_iterator = const std::list<std::pair<Symbol, T>>*;

  Table(size_t capacity = 8)
    : capacity(capacity)
  {
    assert(capacity > 0);
    table = std::make_unique<std::list<std::pair<Symbol, T>>[]>(capacity);
  }

  void enter(const Symbol& s, const T& value)
  {
    if(count + 1 > capacity * load_factor) {
      grow();
    }
    size_t index = s.id() % capacity;
    table[index].emplace_front(s, value);
    count++;
  }

  std::optional<T> lookup(const Symbol& s) const
  {
    size_t index = s.id() % capacity;
    auto iter = std::find_if(table[index].begin(),
                             table[index].end(),
                             [&s](const std::pair<Symbol, T>& pair) -> bool {
                               return std::get<0>(pair).id() == s.id();
                             });

    if(iter == table[index].end()) {
      return std::nullopt;
    }
    return std::get<1>(*iter);
  }

  void pop(const Symbol& s)
  {
    size_t index = s.id() % capacity;
    assert(table[index].size() > 0);
    std::pair<Symbol, T>& front = table[index].front();
    assert(std::get<0>(front) == s);
    table[index].pop_front();
    count--;
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

  void replace(const Symbol& s, const T& value)
  {
    size_t index = s.id() % capacity;
    auto iter = std::find_if(table[index].begin(),
                             table[index].end(),
                             [&s](const std::pair<Symbol, T>& pair) -> bool {
                               return std::get<0>(pair).id() == s.id();
                             });
    assert(iter != table[index].end());
    (*iter).second = value;
  }

  private:
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
        new_table[bin].emplace_back(s, v);
      }
    }
    table = std::move(new_table);
  }

  float load_factor{0.75};
  std::unique_ptr<std::list<std::pair<Symbol, T>>[]> table;
  size_t capacity{};
  size_t count{};
};

} // namespace symbol