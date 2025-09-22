#pragma once
#include <cassert>
#include <ir/fragment.hpp>
#include <string>

namespace arch
{
inline std::string emit_string(const ir::StringFragment& f)
{
  std::string result;
  // we need to escape the string now
  // TODO: what if we keep it escaped from the beginning and just validate the escape sequences
  auto& input = f.lit;
  for(unsigned char c : input)
  {
    switch(c)
    {
    case '\n':
      result += "\\n";
      break;
    case '\t':
      result += "\\t";
      break;
    case '\\':
      result += "\\\\";
      break;
    case '\'':
      result += "\\\'";
      break;
    default:
      if(std::isprint(c))
      {
        result += c;
      }
      else
      {
        // non-printable: use octal escape
        char buf[5];
        std::snprintf(buf, sizeof(buf), "\\%03o", c);
        result += buf;
      }
      break;
    }
  }

  return std::format("{}:\n"
                     ".asciz \"{}\"\n",
                     f.label.str(),
                     result);
}
} // namespace arch