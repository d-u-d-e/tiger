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
  TempGen(TempGen&&) = delete;
  void operator=(TempGen&&) = delete;
  ~TempGen() = default;

  static auto getInstance() -> TempGen&
  {
    static TempGen t;
    return t;
  }

  static auto new_label() -> Label
  {
    auto lid = getInstance().lid++;
    return getInstance().table.symbol(std::format("L{}", lid));
  }

  static auto named_label(const std::string& s) -> Label
  {
    return getInstance().table.symbol(s);
  }

  static auto new_temp() -> Temp
  {
    auto tid = getInstance().tid++;
    return tid;
  }

  static auto to_string(Temp t) -> std::string
  {
    return std::format("t{}", t);
  }

  private:
  TempGen() = default;

  StringTable table;
  uint32_t lid{0};
  uint32_t tid{0};
};