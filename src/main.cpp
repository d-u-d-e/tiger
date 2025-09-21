#include <codegen/arch.hpp>
#include <cstdlib>
#include <cstring>
#include <generated/config.hpp>
#include <iostream>
#include <ir/canon.hpp>
#include <ir/fragment.hpp>
#include <ir/pretty_printer.hpp>
#include <ir/translator.hpp>
#include <ir/tree.hpp>
#include <lexer/lex.hpp>
#include <liveness.hpp>
#include <optional>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <reg_alloc.hpp>
#include <seman/analyzer.hpp>
#include <seman/escape.hpp>

#ifndef NDEBUG
#  define DEBUG_PRETTY_PRINT_IR 0
#  define DEBUG_PRETTY_PRINT_CANONICALIZED_IR 0
#  define DEBUG_PRETTY_PRINT_BLOCKS 0
#  define DEBUG_PRETTY_PRINT_TRACE 0
#  define DEBUG_PRINT_INSTRUCTIONS_BEFORE_REG_ALLOC 0
#  if CONFIG_WITH_GRAPHVIZ
#    define DEBUG_RENDER_FLOW_GRAPH 0
#    define DEBUG_RENDER_INTERFERENCE_GRAPH 0
#  endif
#  define DEBUG_PRINT_LIVENESS_ANALYSIS_RESULTS 0
#endif

enum class Error
{
  PARSE_ERR,
  USAGE_ERR,
  IO_ERR,
  SEMANT_ERR,
};

void terminal_enter_error()
{
  std::cerr << "\033[1;31m";
}

void terminal_exit_error()
{
  std::cerr << "\033[0m";
}

void output(FILE* ofile,
            std::function<arch::Frame::register_t(const ir::TempGen::Temp&)> mapper,
            std::list<::codegen::assem::Instruction>& instrs,
            const std::string& prologue,
            const std::string& epilogue)
{
  // remove instructions that move a register to itself
  std::string result{prologue};
  helpers::delete_coalesced_moves(instrs, mapper);
  auto print_instr_reg_allocated =
    [&mapper, &result](const std::list<::codegen::assem::Instruction>& instrs) {
      for(auto& i : instrs)
      {
        result += arch::codegen::format(mapper, i);
      }
    };
  print_instr_reg_allocated(instrs);
  result += epilogue + "\n";
  std::fwrite(result.c_str(), 1, result.size(), ofile);
}

void code_gen(FILE* ofile, ir::tree::Stmt&& stmt, arch::Frame& f)
{

  [[maybe_unused]] auto sep = "-----------------------------";

#if DEBUG_PRETTY_PRINT_IR
  ir::tree::PrettyPrinter ir_pretty_printer;
  std::cout << "IR" << "\n";
  std::cout << std::visit(ir_pretty_printer, stmt) << "\n" << sep << "\n";
#endif

  ir::tree::Canon canon;
  auto list = canon.linearize(std::move(stmt));

#if DEBUG_PRETTY_PRINT_IR && DEBUG_PRETTY_PRINT_CANONICALIZED_IR
  std::cout << "Reduced IR" << "\n";
  for(auto& s : list)
  {
    std::string reduced = std::visit(ir_pretty_printer, s);
    std::cout << reduced << "\n";
  }
  std::cout << sep << "\n";
#endif

  auto [blocks, ldone] = canon.basic_blocks(std::move(list));

#if DEBUG_PRETTY_PRINT_IR && DEBUG_PRETTY_PRINT_BLOCKS
  std::cout << "Basic blocks" << "\n";
  for(auto& b : blocks)
  {
    std::cout << "<<<< block start" << "\n";
    for(auto& s : b.stmts)
    {
      std::cout << std::visit(ir_pretty_printer, s) << "\n";
    }
    std::cout << ">>>> block end" << "\n\n";
  }
  std::cout << sep << "\n";
#endif

  auto sched = canon.trace_schedule(std::move(blocks), ldone);

#if DEBUG_PRETTY_PRINT_IR && DEBUG_PRETTY_PRINT_TRACE
  std::cout << "Trace" << "\n";
  for(auto& s : sched)
  {
    std::string irstr = std::visit(ir_pretty_printer, s);
    std::cout << irstr << "\n";
  }
  std::cout << sep << "\n";
#endif

  arch::codegen::MuxMunchGen gen;
  std::list<::codegen::assem::Instruction> all;
  for(auto& s : sched)
  {
    auto v = gen.gen(s);
    std::move(v.begin(), v.end(), std::back_inserter(all));
  }

  f.proc_entry_exit2(all);
  auto [pro, epi] = f.proc_entry_exit3(all);

#if DEBUG_PRINT_INSTRUCTIONS_BEFORE_REG_ALLOC
  auto print_instr = [](const std::list<::codegen::assem::Instruction>& instrs) {
    for(auto& i : instrs)
    {
      std::cout << arch::codegen::format(helpers::map_temp, i);
    }
  };
  std::cout << pro;
  print_instr(all);
  std::cout << epi;
  std::cout << sep << "\n";
#endif

  // Create the control flow graph
  auto flow_g = std::make_shared<flow::FlowGraph>(all);

#if DEBUG_RENDER_FLOW_GRAPH
  std::string name = f.name().str() + "_flow";
  flow_g->render(name, name);
#endif

  liveness::LivenessAnalyzer analyzer(*flow_g);
#if DEBUG_PRINT_LIVENESS_ANALYSIS_RESULTS
  std::cout << analyzer.dump_result() << sep << "\n";
#endif

  // Create the register allocator
  register_allocator::RegisterAllocator allocator(flow_g);

#if DEBUG_RENDER_INTERFERENCE_GRAPH
  name = f.name().str() + "_interference";
  allocator.render_igraph_dot(name, name);
#endif

  allocator.perform_allocation();
  auto color_map = allocator.get_color_mapping();

  output(ofile, color_map, all, pro, epi);
}

std::string strip_extension(const std::string& filename)
{
  size_t dot = filename.find_last_of('.');
  if(dot == std::string::npos)
  {
    return filename; // no extension
  }
  return filename.substr(0, dot);
}

std::optional<Error> compile(const std::filesystem::path& source, const char* oname = nullptr)
{
  lexer::Scanner scanner(source);
  symbol::StringTable string_table;
  parser::Parser parser(std::cerr, scanner, string_table);

  terminal_enter_error();
  auto exp = parser.parse();
  terminal_exit_error();

  if(parser.had_error())
  {
    return Error::PARSE_ERR;
  }

  // find escape variables
  seman::EscapeFinder esc_finder;
  exp->accept(esc_finder);

  ir::Translator translator;
  seman::Analyzer type_checker(string_table, translator);
  ir::Exp ir;
  try
  {
    ir = type_checker.type_check(*exp);
  }
  catch(std::exception& e)
  {
    terminal_enter_error();
    std::cerr << e.what() << "\n";
    terminal_exit_error();
    return Error::SEMANT_ERR;
  }

  translator.translate_main_program(std::move(ir));

  std::string in_name = source.filename().string();
  std::string out_name = oname ? oname : strip_extension(in_name) + ".s";
  auto out_file = fopen(out_name.c_str(), "w");

  if(!out_file)
  {
    terminal_enter_error();
    std::cerr << std::format("tigerc: could not write assembly output for {}\n", in_name);
    terminal_exit_error();
    return Error::IO_ERR;
  }

  // dump procedure fragments
  for(auto& frag : translator.fragments())
  {
    if(std::holds_alternative<ir::ProcedureFragment>(frag))
    {
      auto& pf = std::get<ir::ProcedureFragment>(frag);
      code_gen(out_file, std::move(pf.body), *pf.level->frame);
    }
  }

  std::string ending = ".section .note.GNU-stack,\"\",@progbits\n";
  std::fwrite(ending.c_str(), 1, ending.size(), out_file);
  fclose(out_file);
  return std::nullopt;
}

int main(int argc, char** argv)
{
  constexpr int RC_OK{0};
  constexpr int RC_NO_INPUT_ERR{1};
  constexpr int RC_USAGE_ERR{2};
  constexpr int RC_COMPILE_ERR{3};

  const char* oname{nullptr};
  std::vector<std::filesystem::path> input_files;
  std::filesystem::path input;
  std::vector<std::filesystem::path> link_dir;
  for(int i = 1; i < argc; i++)
  {
    if(strcmp(argv[i], "-o") == 0 && (i + 1) < argc)
    {
      oname = argv[i + 1];
      i += 1;
    }
    else
    {
      // input is considered a file to be processed
      input_files.emplace_back(argv[i]);
    }
  }

  if(input_files.empty())
  {
    terminal_enter_error();
    std::cerr << "tigerc: no input files\n";
    terminal_exit_error();
    return RC_NO_INPUT_ERR;
  }

  if(oname && input_files.size() > 1)
  {
    terminal_enter_error();
    std::cerr << "tigerc: cannot specify '-o' with multiple input files\n";
    terminal_exit_error();
    return RC_USAGE_ERR;
  }

  int rc{RC_OK};
  for(const auto& input_file : input_files)
  {
    auto err = compile(input_file, oname);
    if(err.has_value())
    {
      rc = RC_COMPILE_ERR;
    }
  }
  return rc;
}