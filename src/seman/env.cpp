#include <seman/env.hpp>
#include <utility>

namespace seman::env
{

Symbol scope_marker{"", 0};

std::string VEntry::to_string() const
{
  if(std::holds_alternative<VarEntry>(this->v)) {
    return "VarEntry" +
           std::format("({})", std::get<VarEntry>(this->v).type->to_string());
  }
  else if(std::holds_alternative<FuncEntry>(this->v)) {
    std::string result = "FuncEntry(";
    auto& formals = std::get<FuncEntry>(this->v).formals;
    size_t size = formals.size();
    for(size_t i = 0; i < size; i++) {
      result += formals[i]->to_string() + (i == size - 1 ? "" : ", ");
    }
    return result + ") -> " + std::get<FuncEntry>(this->v).result->to_string();
  }
  assert(false);
  std::unreachable();
}

std::string TEntry::to_string() const
{
  return this->t->to_string();
}

} // namespace seman::env