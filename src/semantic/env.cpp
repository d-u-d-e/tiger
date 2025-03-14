#include <semantic/env.hpp>
#include <utility>

namespace semantic::env
{

std::string to_string(const VEntry& entry)
{
  if(std::holds_alternative<VarEntry>(entry)) {
    return "VarEntry" +
           std::format("({})", std::get<VarEntry>(entry).type->to_string());
  }
  else if(std::holds_alternative<FuncEntry>(entry)) {
    std::string result = "FuncEntry(";
    auto& formals = std::get<FuncEntry>(entry).formals;
    size_t size = formals.size();
    for(size_t i = 0; i < size; i++) {
      result += formals[i]->to_string() + (i == size - 1 ? "" : ", ");
    }
    return result + ") -> " + std::get<FuncEntry>(entry).result->to_string();
  }
  assert(false);
  std::unreachable();
}

std::string to_string(const TEntry& entry)
{
  return entry.t->to_string();
}

} // namespace semantic::env