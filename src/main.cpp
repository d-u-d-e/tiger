#include <algorithm>
#include <codegen/arch.hpp>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <ostream>
#include <reg_alloc.hpp>

#include <codegen/assem.hpp>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <iostream>
#include <ir/fragment.hpp>
#include <ir/tree.hpp>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

#include <seman/analyzer.hpp>
#include <seman/env.hpp>
#include <symbol.hpp>
#include <sysexits.h>

#include <ir/pretty_printer.hpp>
#include <ir/translator.hpp>
#include <seman/escape.hpp>
#include <utility>

#include <flow.hpp>
#include <ir/canon.hpp>
#include <liveness.hpp>
#include <variant>
#include <vector>

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

  auto sep = "-----------------------------";
  ir::tree::PrettyPrinter ir_pretty_printer;
  (void)ir_pretty_printer;

  /*
  std::cout << "IR" << "\n";
  std::cout << std::visit(ir_pretty_printer, stmt) << "\n" << sep << "\n";
  */

  ir::tree::Canon canon;
  auto list = canon.linearize(std::move(stmt));

  /*
  std::cout << "Reduced" << "\n";
  for(auto& s : list) {
    std::string reduced = std::visit(ir_pretty_printer, s);
    std::cout << reduced << "\n";
  }
  std::cout << sep << "\n";
  */

  auto [blocks, ldone] = canon.basic_blocks(std::move(list));

  /*
  std::cout << "Basic blocks" << "\n";
  for(auto& b : blocks) {
    std::cout << "<<<< block start" << "\n";
    for(auto& s : b.stmts) {
      std::string irstr = std::visit(ir_pretty_printer, s);
      std::cout << irstr << "\n";
    }
    std::cout << ">>>> block end" << "\n" << "\n";
  }
  std::cout << sep << "\n";
  */

  auto sched = canon.trace_schedule(std::move(blocks), ldone);

  /* 
  std::cout << "Trace" << "\n";
  for(auto& s : sched) {
    std::string irstr = std::visit(ir_pretty_printer, s);
    std::cout << irstr << "\n";
  }
  std::cout << sep << "\n";
  */

  arch::codegen::MuxMunchGen gen;
  std::list<::codegen::assem::Instruction> all;
  for(auto& s : sched)
  {
    auto v = gen.gen(s);
    std::move(v.begin(), v.end(), std::back_inserter(all));
  }

  f.proc_entry_exit2(all);
  auto [pro, epi] = f.proc_entry_exit3(all);

  /*
  auto print_instr = [](const std::list<::codegen::assem::Instruction>& instrs) {
    for(auto& i : instrs)
    {
      std::cout << arch::codegen::format(helpers::map_temp, i);
    }
  };

  std::cout << pro;
  print_instr(all);
  std::cout << epi;
  std::cout << sep << "\n";*/

  // Create the control flow graph
  auto flow_g = std::make_shared<flow::FlowGraph>(all);
  std::string name = f.name().str() + "_flow";
  flow_g->render(name, name);

  liveness::LivenessAnalyzer analyzer(*flow_g);
  std::cout << analyzer.dump_result() << "\n";
  std::cout << sep << "\n";

  // Create the register allocator
  register_allocator::RegisterAllocator allocator(flow_g);

  /*name = f.name().str() + "_interference";
  allocator.render_igraph_dot(name, name);*/

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

int main(int argc, char** argv)
{
  const char* oname{nullptr};
  std::vector<std::filesystem::path> input_files;
  std::vector<std::filesystem::path> link_dir;
  for(int i = 1; i < argc; i++)
  {
    if(strcmp(argv[i], "-o") == 0 && (i + 1) < argc)
    {
      oname = argv[i + 1];
    }
    else
    {
      // input is considered a file to be processed
      input_files.emplace_back(argv[i]);
    }
  }

  if(input_files.empty())
  {
    std::cerr << "\033[1;31m";
    std::cerr << "tigerc: no input files\n";
    std::cerr << "\033[0m";
    return EX_NOINPUT;
  }

  if(!oname)
  {
    oname = "a.out";
  }

  // TODO: do all, here we do the first only
  std::filesystem::path input = input_files[0];

  lexer::Scanner scanner(input);
  symbol::StringTable string_table;
  parser::Parser parser(std::cerr, scanner, string_table);

  std::cerr << "\033[1;31m";
  auto exp = parser.parse();
  std::cerr << "\033[0m";

  if(parser.had_error())
  {
    return EX_DATAERR;
  }

  /*
  // pretty print the ast
  parser::ast::PrettyPrinter pretty_printer;
  std::cout << exp->accept(pretty_printer) << "\n" << "\n";

  // dump the string table
  std::cout << "string table:" << "\n";
  std::cout << string_table.dump() << "\n";
  */

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
    std::cerr << "\033[1;31m" << e.what() << "\033[0m" << "\n";
    return EX_DATAERR;
  }

  translator.translate_main_program(std::move(ir));

  // TODO: should we move this to a python script that acts as the driver?
  // this way we remove all the system calls

  // create a tmp file to write the assembly
  std::string input_fname =
    std::string("/tmp/") + strip_extension(input.filename().string()) + ".s";

  auto tmp_file = fopen(input_fname.c_str(), "w");
  if(!tmp_file)
  {
    std::cerr << "\033[1;31m" << "tigerc: could not write assembly output" << "\033[0m" << "\n";
    return EX_UNAVAILABLE;
  }

  // dump procedure fragments
  for(auto& frag : translator.fragments())
  {
    if(std::holds_alternative<ir::ProcedureFragment>(frag))
    {
      auto& pf = std::get<ir::ProcedureFragment>(frag);
      code_gen(tmp_file, std::move(pf.body), *pf.level->frame);
    }
  }

  std::string ending = ".section .note.GNU-stack,\"\",@progbits\n";
  std::fwrite(ending.c_str(), 1, ending.size(), tmp_file);

  // close the tmp file
  fclose(tmp_file);

  // we need to assemble the temporary file
  std::string tmp_object_fname = input_fname + ".o";
  auto cmd = std::format("gcc -c -o {} {} > /dev/null 2>&1", tmp_object_fname, input_fname);
  std::system(cmd.c_str());

  // see if the file exists
  if(!std::filesystem::exists(tmp_object_fname))
  {
    std::cerr << "\033[1;31m" << "tigerc: could not run assembler" << "\033[0m" << "\n";
    return EX_UNAVAILABLE;
  }

  // find out where we are
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  auto mypath = std::string(result, (count > 0) ? count : 0);
  auto runtime_path = std::filesystem::path(mypath).parent_path().string() + "/../lib/runtime.o";

  // we need to link against runtime.o
  cmd = std::format("gcc {} {} -o {} > /dev/null 2>&1", runtime_path, tmp_object_fname, oname);
  std::system(cmd.c_str());

  // see if the file exists
  if(!std::filesystem::exists(oname))
  {
    std::cerr << "\033[1;31m" << "tigerc: could not run linker" << "\033[0m" << "\n";
    return EX_UNAVAILABLE;
  }

  return EX_OK;
}