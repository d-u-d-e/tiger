#pragma once
#include <codegen/arch.hpp>
#include <ir/temp.hpp>
#include <list>

namespace helpers
{
std::list<ir::TempGen::Temp> union_sorted_lists(const std::list<ir::TempGen::Temp>& a,
                                                const std::list<ir::TempGen::Temp>& b);
std::list<ir::TempGen::Temp> diff_sorted_lists(const std::list<ir::TempGen::Temp>& a,
                                               const std::list<ir::TempGen::Temp>& b);
std::string map_temp(const ir::TempGen::Temp& t);

} // namespace helpers