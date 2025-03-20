#pragma once
#include <format>
#include <symbol.hpp>

namespace ir
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
    return getInstance().table.symbol(std::format("L{}", lid));
  }

  static label_t named_label(const std::string& s)
  {
    return getInstance().table.symbol(s);
  }

  static temp_t new_temp()
  {
    auto tid = getInstance().tid++;
    return tid;
  }

  private:
  Temp() = default;

  symbol::StringTable table;
  uint32_t lid{0};
  uint32_t tid{0};
};

} // namespace ir