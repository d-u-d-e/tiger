#include "compiler.hpp"
#include "ir/translator.hpp"
#include "parser/parser.hpp"
#include "semant/analyzer.hpp"
#include "semant/escape.hpp"
#include "string_table.hpp"
#include "target.hpp"
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

  using Translator = ir::Translator<FrameImpl>;
  Translator translator;
  semant::Analyzer<Translator> type_checker(source, string_table, translator);
  ir::Exp ir;
  try
  {
    ir = type_checker.type_check(*exp);
  }
  catch(semant::Exception& e)
  {
    terminal_enter_error();
    std::println(std::cerr, "{}", e.what());
    terminal_exit_error();
    return Error::SEMAN_ERR;
  }

  // AST to IR
  translator.translate_main_program(std::move(ir));
  auto out_file = fopen(out_name.c_str(), "w");
  if(!out_file)
  {
    terminal_enter_error();
    std::println(std::cerr, "tigerc: could not write assembly output for {}", source.string());
    terminal_exit_error();
    return Error::IO_ERR;
  }

  // IR to Assembly
  
  // TODO
  return std::nullopt;
}