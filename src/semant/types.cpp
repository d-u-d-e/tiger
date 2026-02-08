#include "semant/types.hpp"
#include <cstddef>
#include <string>

namespace semant::types
{

std::string to_string(const SharedType& t)
{
  return t->to_string();
}

std::string Integer::to_string()
{
  return "int";
}

std::string String::to_string()
{
  return "string";
}

std::string Nil::to_string()
{
  return "nil";
}

std::string Unit::to_string()
{
  return "unit";
}

std::string Record::to_string()
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

std::string Array::to_string()
{
  return "arr of " + type->to_string();
}

std::string Name::to_string()
{
  return name.str();
}

std::string FunctionType::to_string()
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
