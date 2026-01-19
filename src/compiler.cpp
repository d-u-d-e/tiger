#include "compiler.hpp"
#include "parser/parser.hpp"
#include "semant/escape.hpp"
#include "string_table.hpp"
#include "terminal.hpp"
#include <optional>

namespace
{

std::string strip_extension(const std::string& filename)
{
  size_t dot = filename.find_last_of('.');
  if(dot == std::string::npos)
  {
    return filename; // no extension
  }
  return filename.substr(0, dot);
}

} // namespace

template <typename Target>
auto make_translator()
{
  
}

std::optional<Compiler::Error> Compiler::compile(const std::filesystem::path& source,
                                                 const char* oname)
{
  StringTable string_table;
  std::string out_name = oname ? oname : strip_extension(source.filename().string()) + ".s";

  std::unique_ptr<parser::ast::Expression> exp;
  try
  {
    terminal_enter_error();
    lexer::Scanner scanner(source);
    parser::Parser parser(std::cerr, scanner, string_table);
    exp = parser.parse();

    // the parser does not stop at the first error
    if(parser.had_error())
    {
      terminal_exit_error();
      return Error::PARSE_ERR;
    }
    terminal_exit_error();
  }
  catch(lexer::Exception& e)
  {
    std::println(std::cerr, "{}", e.what());
    terminal_exit_error();
    return Error::LEX_ERR;
  }

  // find escape variables
  semant::EscapeFinder esc_finder;
  exp->accept(esc_finder);

  return std::nullopt;
}