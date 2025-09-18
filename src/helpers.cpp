#include <helpers.hpp>

namespace helpers
{

std::string map_temp(const ir::TempGen::Temp& t)
{
  auto mapped = arch::Frame::map_temp(t);
  if(mapped)
  {
    return mapped.value();
  }
  return ir::TempGen::to_string(t);
};

} // namespace helpers