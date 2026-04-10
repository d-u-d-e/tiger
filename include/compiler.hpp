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
  static auto compile(const std::filesystem::path& source, const char* oname = nullptr)
    -> std::optional<Error>;

  private:
  static auto linearize_tree(ir::tree::Stmt&& stmt) -> std::list<ir::tree::Stmt>;
  static auto strip_extension(const std::string& filename) -> std::string;
};