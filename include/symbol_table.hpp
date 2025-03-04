#pragma once
#include <list>
#include <memory>
#include <optional>
#include <symbol.hpp>

namespace symbol
{

template <typename T>
class SymbolTable {
  public:
  SymbolTable() = default;

  void enter(const Symbol& s, T&& value)
  {

    if(count + 1 > capacity * load_factor) {
      grow();
    }

    size_t index = s.id() % capacity;
    table[index].emplace_front(s, value);
  }

  std::optional<T> lookup(const Symbol& s)
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

  private:
  void grow()
  {
    auto capacity_old = capacity;
    capacity = (capacity < 8) ? 8 : 2 * capacity;
    std::unique_ptr<std::list<std::pair<Symbol, T>>[]> new_table =
      std::make_unique<std::list<std::pair<Symbol, T>>[]>(capacity);

    for(size_t i = 0; i < capacity_old; i++) {
      auto& l = table[i];
      for(auto& [s, v] : l) {
        auto bin = s.id() & capacity;
        new_table[bin].emplace_front(s, v);
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