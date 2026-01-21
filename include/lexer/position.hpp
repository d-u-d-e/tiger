#pragma once
#include <format>
#include <string>

namespace lexer
{

struct Position
{
  int line;
  int column;

  bool operator==(const Position& other) const
  {
    return line == other.line && column == other.column;
  }

  std::string to_string() const
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