#pragma once
#include "string_table.hpp"
#include "symbol.hpp"
#include <cstdint>
#include <format>
#include <string>

class TempGen
{
  public:
  using Label = Symbol;
  using Temp = uint32_t;

  TempGen(const TempGen&) = delete;
  void operator=(const TempGen&) = delete;

  static TempGen& getInstance()
  {
    static TempGen t;
    return t;
  }

  static Label new_label()
  {
    auto lid = getInstance().lid++;
    return getInstance().table.symbol(std::format("L{}", lid));
  }

  static Label named_label(const std::string& s)
  {
    return getInstance().table.symbol(s);
  }

  static Temp new_temp()
  {
    auto tid = getInstance().tid++;
    return tid;
  }

  static std::string to_string(Temp t)
  {
    return std::format("t{}", t);
  }

  private:
  TempGen() = default;

  StringTable table;
  uint32_t lid{0};
  uint32_t tid{0};
};