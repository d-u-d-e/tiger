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

void delete_coalesced_moves(
  std::list<::codegen::assem::Instruction>& instrs,
  const std::function<arch::Frame::register_t(const ir::TempGen::Temp&)>& reg_mapper)
{
  auto iter =
    std::remove_if(instrs.begin(), instrs.end(), [&reg_mapper](::codegen::assem::Instruction& i) {
      if(std::holds_alternative<::codegen::assem::Move>(i))
      {
        auto m = std::get<::codegen::assem::Move>(i);
        if(reg_mapper(m.src) == reg_mapper(m.dst))
        {
          return true;
        }
      }
      return false;
    });
  instrs.erase(iter, instrs.end());
}

} // namespace helpers