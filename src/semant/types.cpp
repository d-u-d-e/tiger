#include "semant/types.hpp"
#include <cstddef>
#include <string>

namespace semant::types
{

auto to_string(const SharedType& t) -> std::string
{
  return t->to_string();
}

auto Integer::to_string() -> std::string
{
  return "int";
}

auto String::to_string() -> std::string
{
  return "string";
}

auto Nil::to_string() -> std::string
{
  return "nil";
}

auto Unit::to_string() -> std::string
{
  return "unit";
}

auto Record::to_string() -> std::string
{
  std::string result = "{";
  auto size = fields.size();

  for(size_t i = 0; i < size; i++)
  {
    auto& [name, type] = fields[i];
    result += name.str() + ": " + type->to_string() + (i == size - 1 ? "" : ", ");
  }
  return result + "}";
}

auto Array::to_string() -> std::string
{
  return "arr of " + type->to_string();
}

auto Name::to_string() -> std::string
{
  return name.str();
}

auto FunctionType::to_string() -> std::string
{
  std::string result = "{(";
  auto size = formals.size();

  for(size_t i = 0; i < size; i++)
  {
    result += formals[i]->to_string() + (i == size - 1 ? "" : ", ");
  }
  result += ") -> " + ret->to_string() + "}";
  return result;
}

} // namespace semant::types
