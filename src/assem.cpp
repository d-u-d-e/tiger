#include "assem.hpp"

namespace assem
{

std::string format(std::function<std::string(const TempGen::Temp& t)> mapper,
                   const Instruction& ins)
{

  auto replace_placeholders = [&mapper](const std::vector<TempGen::Temp>& src,
                                        const std::vector<TempGen::Temp>& dst,
                                        const std::string& assem) {
    std::string result;
    for(size_t i = 0; i < assem.length(); i++)
    {
      if(std::string_view(assem.data() + i, 2) == "`s")
      {
        size_t pos;
        auto ix = std::stoi(assem.substr(i + 2), &pos);
        result += mapper(src[ix]);
        i += pos + 1;
      }
      else if(std::string_view(assem.data() + i, 2) == "`d")
      {
        size_t pos;
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
    auto& cins = std::get<Oper>(ins);
    return replace_placeholders(cins.src, cins.dst, cins.assem);
  }
  if(std::holds_alternative<Move>(ins))
  {
    auto& cins = std::get<Move>(ins);
    return replace_placeholders({cins.src}, {cins.dst}, cins.assem);
  }
  else if(std::holds_alternative<Label>(ins))
  {
    return std::get<Label>(ins).assem;
  }
  return "?\n";
}

} // namespace assem