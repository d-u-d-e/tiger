#include <semantic/types.hpp>

namespace semantic::types
{

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

  for(auto i = 0; i < size; i++) {
    auto& [name, type] = fields[i];
    result +=
      name.name() + ": " + type->to_string() + (i == size - 1 ? "" : ", ");
  }
  return result + "}";
}

std::string Array::to_string()
{
  return "arr of " + type->to_string();
}

std::string Name::to_string()
{
  return name.name();
}

} // namespace semantic::types
