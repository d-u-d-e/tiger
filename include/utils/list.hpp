#pragma once
#include <list>

namespace utils
{
template <typename T>
auto diff_sorted_lists(const std::list<T>& a, const std::list<T>& b) -> std::list<T>
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
auto union_sorted_lists(const std::list<T>& a, const std::list<T>& b) -> std::list<T>
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
} // namespace utils