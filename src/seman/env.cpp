#include <arch/frame.hpp>
#include <cassert>
#include <cstddef>
#include <format>
#include <seman/env.hpp>
#include <string>
#include <symbol.hpp>
#include <utility>
#include <variant>

namespace seman::env
{

Symbol scope_marker{"", 0};

std::string VEntry::to_string() const
{
  if(std::holds_alternative<VarEntry>(this->v)) {
    auto& v = std::get<VarEntry>(this->v);
    return "VarEntry" + std::format("(t: {}, ax: {})",
                                    v.type->to_string(),
                                    arch::Frame::to_string(v.access.fax));
  }
  else if(std::holds_alternative<FuncEntry>(this->v)) {
    auto& f = std::get<FuncEntry>(this->v);
    std::string result = std::format("FuncEntry({}) (", f.label.str());
    size_t size = f.formals.size();
    for(size_t i = 0; i < size; i++) {
      result += f.formals[i]->to_string() + (i == size - 1 ? "" : ", ");
    }
    return result + ") -> " + f.result->to_string();
  }
  assert(false);
  std::unreachable();
}

std::string TEntry::to_string() const
{
  return this->t->to_string();
}

} // namespace seman::env