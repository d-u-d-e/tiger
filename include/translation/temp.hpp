#pragma once
#include <format>
#include <symbol.hpp>

namespace translation
{

class Temp {
  public:
  using label_t = symbol::Symbol;
  using temp_t = uint32_t;

  Temp(const Temp&) = delete;
  void operator=(const Temp&) = delete;

  static Temp& getInstance()
  {
    static Temp t;
    return t;
  }

  static label_t new_label()
  {
    auto lid = getInstance().lid++;
    return symbol::Symbol(std::format("L{}", lid), lid);
  }

  static label_t named_label(const std::string& s)
  {
    auto lid = getInstance().lid++;
    return symbol::Symbol(s, lid);
  }

  static temp_t new_temp()
  {
    auto tid = getInstance().tid++;
    return tid;
  }

  private:
  Temp() = default;

  uint32_t lid{0};
  uint32_t tid{0};
};

} // namespace translation