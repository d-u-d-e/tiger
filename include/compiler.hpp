#pragma once
#include "ir/tree.hpp"
#include <filesystem>
#include <list>
#include <optional>

class Compiler
{
  public:
  enum class Error
  {
    LEX_ERR,
    PARSE_ERR,
    SEMANT_ERR,
    USAGE_ERR,
    IO_ERR,
  };
  std::optional<Error> compile(const std::filesystem::path& source, const char* oname = nullptr);

  private:
  static std::list<ir::tree::Stmt> linearize_tree(ir::tree::Stmt&& stmt);
  static std::string strip_extension(const std::string& filename);
};