#include <format>
#include <symbol.hpp>

namespace symbol
{
const Symbol& SymbolTable::symbol(const std::string& name)
{
  if(table.find(name) == table.end()) {
    table.emplace(name, Symbol(name, identifier++));
  }
  return table.at(name);
}

std::string SymbolTable::dump() const
{
  std::string result;
  auto constexpr col1_width = 20;
  auto constexpr col2_width = 5;

  result = std::format("{:<{}} | {:<{}}\n", "Name", col1_width, "Identifier", col2_width);
  result += std::string(col1_width + col2_width + 3, '-') + '\n';

  for(const auto& [name, symbol] : table) {
    result +=
      std::format("{:<{}} | {:<{}}\n", name, col1_width, symbol.id(), col2_width);
  }
  return result;
}

} // namespace symbol