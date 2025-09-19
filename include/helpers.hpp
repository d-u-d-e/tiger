#pragma once
#include <codegen/arch.hpp>
#include <list>

namespace helpers
{

template <typename T>
std::list<T> diff_sorted_lists(const std::list<T>& a, const std::list<T>& b)
{
  std::list<T> out;
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

template <typename T>
std::list<T> union_sorted_lists(const std::list<T>& a, const std::list<T>& b)
{
  std::list<T> out;
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
std::string map_temp(const ir::TempGen::Temp& t);

void delete_coalesced_moves(
  std::list<::codegen::assem::Instruction>& instrs,
  const std::function<arch::Frame::register_t(const ir::TempGen::Temp&)>& reg_mapper);

} // namespace helpers