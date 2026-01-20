#pragma once
#include <filesystem>
#include <optional>

class Compiler
{
  public:
  enum class Error
  {
    LEX_ERR,
    PARSE_ERR,
    SEMAN_ERR,
    USAGE_ERR,
    IO_ERR,
  };
  std::optional<Error> compile(const std::filesystem::path& source, const char* oname = nullptr);
};