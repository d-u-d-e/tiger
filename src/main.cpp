#include "compiler.hpp"
#include "terminal.hpp"
#include <filesystem>
#include <vector>

auto main(int argc, char** argv) -> int
{
  constexpr int RC_OK{0};
  constexpr int RC_NO_INPUT_ERR{1};
  constexpr int RC_USAGE_ERR{2};
  constexpr int RC_COMPILE_ERR{3};

  const char* oname{nullptr};
  std::vector<std::filesystem::path> input_files;
  auto args = std::span(argv, size_t(argc));

  for(int i = 1; i < argc; i++)
  {
    if(std::string_view(args[i]) == "-o" && (i + 1) < argc)
    {
      oname = args[i + 1];
      i += 1;
    }
    else
    {
      // input is considered a file to be processed
      input_files.emplace_back(args[i]);
    }
  }

  if(input_files.empty())
  {
    terminal_write_error("tigerc: no input files");
    return RC_NO_INPUT_ERR;
  }

  if((oname != nullptr) && input_files.size() > 1)
  {
    terminal_write_error("tigerc: cannot specify '-o' with multiple input files");
    return RC_USAGE_ERR;
  }

  int rc{RC_OK};
  const Compiler compiler;

  for(const auto& input_file : input_files)
  {
    auto err = compiler.compile(input_file, oname);
    if(err.has_value())
    {
      rc = RC_COMPILE_ERR;
    }
  }
  return rc;
}
