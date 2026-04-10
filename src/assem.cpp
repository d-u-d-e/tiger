#include "assem.hpp"

#include <algorithm>

namespace assem
{

auto format(std::function<register_t(const TempGen::Temp& t)> mapper, const Instruction& ins)
  -> std::string
{

  auto replace_placeholders = [&mapper](const std::vector<TempGen::Temp>& src,
                                        const std::vector<TempGen::Temp>& dst,
                                        const std::string& assem) {
    std::string result;
    for(size_t i = 0; i < assem.length(); i++)
    {
      if(std::string_view(&assem[i], 2) == "`s")
      {
        size_t pos{};
        auto ix = std::stoi(assem.substr(i + 2), &pos);
        result += mapper(src[ix]);
        i += pos + 1;
      }
      else if(std::string_view(&assem[i], 2) == "`d")
      {
        size_t pos{};
        auto ix = std::stoi(assem.substr(i + 2), &pos);
        result += mapper(dst[ix]);
        i += pos + 1;
      }
      else
      {
        result += assem[i];
      }
    }
    return result;
  };

  if(std::holds_alternative<Oper>(ins))
  {
    const auto& cins = std::get<Oper>(ins);
    return replace_placeholders(cins.src, cins.dst, cins.assem);
  }
  if(std::holds_alternative<Move>(ins))
  {
    const auto& cins = std::get<Move>(ins);
    return replace_placeholders({cins.src}, {cins.dst}, cins.assem);
  }
  else if(std::holds_alternative<Label>(ins))
  {
    return std::get<Label>(ins).assem;
  }
  return "?\n";
}

void delete_coalesced_moves(std::list<Instruction>& instrs,
                            const std::function<register_t(const TempGen::Temp&)>& reg_mapper)
{
  auto ret = std::ranges::remove_if(instrs, [&reg_mapper](assem::Instruction& i) {
    if(std::holds_alternative<assem::Move>(i))
    {
      auto m = std::get<assem::Move>(i);
      if(reg_mapper(m.src) == reg_mapper(m.dst))
      {
        return true;
      }
    }
    return false;
  });
  instrs.erase(ret.begin(), ret.end());
}

} // namespace assem