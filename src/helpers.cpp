#include <helpers.hpp>

namespace helpers
{

std::list<ir::TempGen::Temp> diff_sorted_lists(const std::list<ir::TempGen::Temp>& a,
                                               const std::list<ir::TempGen::Temp>& b)
{
  std::list<ir::TempGen::Temp> out;
  auto itera = a.cbegin();
  auto iterb = b.cbegin();
  while(itera != a.end() && iterb != b.end())
  {
    if(*itera == *iterb)
    {
      itera++;
      iterb++;
    }
    else if(*itera < *iterb)
    {
      out.emplace_back(*itera);
      itera++;
    }
    else
    {
      iterb++;
    }
  }

  out.insert(out.end(), itera, a.end());
  return out;
}

std::list<ir::TempGen::Temp> union_sorted_lists(const std::list<ir::TempGen::Temp>& a,
                                                const std::list<ir::TempGen::Temp>& b)
{
  std::list<ir::TempGen::Temp> out;
  auto itera = a.cbegin();
  auto iterb = b.cbegin();
  while(itera != a.end() && iterb != b.end())
  {
    if(*itera == *iterb)
    {
      out.emplace_back(*itera);
      itera++;
      iterb++;
    }
    else if(*itera < *iterb)
    {
      out.emplace_back(*itera);
      itera++;
    }
    else
    {
      out.emplace_back(*iterb);
      iterb++;
    }
  }

  out.insert(out.end(), itera, a.end());
  out.insert(out.end(), iterb, b.end());
  return out;
}

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