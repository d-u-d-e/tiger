#include "string_table.hpp"
#include <format>

auto StringTable::symbol(const std::string& name) -> const Symbol&
{
  if(!table.contains(name))
  {
    table.emplace(name, Symbol(name, identifier++));
  }
  return table.at(name);
}

auto StringTable::dump() const -> std::string
{
  std::string result;
  auto constexpr col1_width = 20;
  auto constexpr col2_width = 5;

  result = std::format("{:<{}} | {:<{}}\n", "name", col1_width, "id", col2_width);
  result += std::string(col1_width + col2_width + 3, '-') + '\n';

  for(const auto& [name, symbol] : table)
  {
    result += std::format("{:<{}} | {:<{}}\n", name, col1_width, symbol.id(), col2_width);
  }
  return result;
}