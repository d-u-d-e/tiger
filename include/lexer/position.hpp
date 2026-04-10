#pragma once
#include <format>
#include <string>

namespace lexer
{

struct Position
{
  int line;
  int column;

  auto operator==(const Position& other) const -> bool
  {
    return line == other.line && column == other.column;
  }

  auto to_string() const -> std::string
  {
    return std::to_string(line) + ":" + std::to_string(column);
  }
};

} // namespace lexer

template <>
struct std::formatter<lexer::Position> : std::formatter<std::string>
{
  auto format(lexer::Position p, format_context& ctx) const
  {
    return formatter<string>::format(p.to_string(), ctx);
  }
};