#pragma once
#include <codegen/arch.hpp>
#include <ir/temp.hpp>

namespace helpers
{
inline auto map_temp(const ir::TempGen::Temp& t)
{
  auto mapped = arch::Frame::map_temp(t);
  if(mapped)
  {
    return mapped.value();
  }
  return ir::TempGen::to_string(t);
};
} // namespace helpers