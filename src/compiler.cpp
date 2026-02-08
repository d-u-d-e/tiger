#include "compiler.hpp"
#include "assem.hpp"
#include "flow.hpp"
#include "ir/canon.hpp"
#include "ir/fragment.hpp"
#include "ir/translator.hpp"
#include "ir/tree.hpp"
#include "liveness.hpp"
#include "parser/parser.hpp"
#include "register_allocator.hpp"
#include "semant/analyzer.hpp"
#include "semant/escape.hpp"
#include "string_table.hpp"
#include "target.hpp"
#include "temp.hpp"
#include "terminal.hpp"
#include <optional>
#include <print>
#include <variant>

#ifndef NDEBUG
#  define DEBUG_PRETTY_PRINT_AST 0
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
#  define DEBUG_PRINT_INSTRUCTIONS_ON_SPILLING 0
#endif

#if DEBUG_PRETTY_PRINT_CANONICALIZED_IR || DEBUG_PRETTY_PRINT_BLOCKS || DEBUG_PRETTY_PRINT_TRACE
#  include <concepts>
#endif

#if DEBUG_PRETTY_PRINT_AST
#  include "parser/pretty_printer.hpp"
#endif

namespace
{

[[maybe_unused]] auto constexpr sep = "-----------------------------";

std::string temporary_mapper(const TempGen::Temp& t)
{
  auto reg_map = FrameImpl::get_temporary_register_mapping();

  if(reg_map.find(t) != reg_map.end())
  {
    return reg_map[t];
  }
  return TempGen::to_string(t);
}

void write_instructions(FILE* ofile,
                        std::function<assem::register_t(const TempGen::Temp&)> mapper,
                        std::list<assem::Instruction>& instrs,
                        std::string prologue,
                        std::string epilogue)
{
  // remove instructions that move a register to itself
  std::string result{std::move(prologue)};
  assem::delete_coalesced_moves(instrs, mapper);
  for(auto& i : instrs)
  {
    result += assem::format(mapper, i);
  }
  result += std::move(epilogue);
  result += +"\n";
  std::fwrite(result.c_str(), 1, result.size(), ofile);
}

void emit_procedure_fragment(FILE* ofile, FrameImpl& f, std::list<ir::tree::Stmt>& body)
{
  // generate target code, with an infinite number of machine registers
  GeneratorImpl gen;
  std::list<assem::Instruction> all;
  for(auto& s : body)
  {
    auto v = gen.gen(s);
    std::move(v.begin(), v.end(), std::back_inserter(all));
  }

  // prepare for register allocation
  f.proc_entry_exit2(all);

#if DEBUG_PRINT_INSTRUCTIONS_BEFORE_REG_ALLOC
  std::println("Assembly before register allocation [frame: {}]", f.name().str());
  for(auto& i : all)
  {
    std::print("{}", assem::format(temporary_mapper, i));
  }
  std::println(sep);
#endif

  bool spilling_required{false};
  do
  {
    // create the control flow graph
    auto flow_g = std::make_shared<flow::FlowGraph>(all, temporary_mapper);

#if DEBUG_RENDER_FLOW_GRAPH
    std::string namef = f.name().str() + "_flow";
    flow_g->render(namef, namef);
#endif

    liveness::Analyzer analyzer(*flow_g);

#if DEBUG_PRINT_LIVENESS_ANALYSIS_RESULTS
    std::println("Results of liveness analysis [frame: {}]", f.name().str());
    std::println("{}{}", analyzer.dump_result(), sep);
#endif

    IteratedRegisterCoalescing allocator(flow_g, FrameImpl::get_temporary_register_mapping());

#if DEBUG_RENDER_INTERFERENCE_GRAPH
    std::string namei = f.name().str() + "_interference";
    allocator.render_igraph_dot(namei, namei);
#endif

    auto spilled_nodes = allocator.perform_allocation();
    spilling_required = !spilled_nodes.empty();

    if(!spilling_required)
    {
      auto color_map = allocator.get_color_mapping();
      // once we know the number of spilled temporaries, we can evaluate the required stack space for the frame
      auto [pro, epi] = f.proc_entry_exit3(all);
      write_instructions(ofile, color_map, all, pro, epi);
    }
    else
    {
      f.rewrite_program(all, spilled_nodes);
#if DEBUG_PRINT_INSTRUCTIONS_ON_SPILLING
      std::print("Spilled temporaries [frame: {}]: ", f.name().str());
      for(auto t : spilled_nodes)
      {
        std::print("{}, ", t);
      }
      std::println("\nAssembly after spilling [frame: {}]", f.name().str());
      for(auto& i : all)
      {
        std::print("{}", assem::format(temporary_mapper, i));
      }
      std::println(sep);
#endif
    }
  } while(spilling_required);
}

} // namespace

std::string Compiler::strip_extension(const std::string& filename)
{
  size_t dot = filename.find_last_of('.');
  if(dot == std::string::npos)
  {
    return filename; // no extension
  }
  return filename.substr(0, dot);
}

std::list<ir::tree::Stmt> Compiler::linearize_tree(ir::tree::Stmt&& stmt)
{
#if DEBUG_PRETTY_PRINT_IR || DEBUG_PRETTY_PRINT_CANONICALIZED_IR || DEBUG_PRETTY_PRINT_BLOCKS ||   \
  DEBUG_PRETTY_PRINT_TRACE
  ir::tree::PrettyPrinter ir_pretty_printer;
#endif

#if DEBUG_PRETTY_PRINT_CANONICALIZED_IR || DEBUG_PRETTY_PRINT_BLOCKS || DEBUG_PRETTY_PRINT_TRACE
  auto pretty_print_stmts = [&ir_pretty_printer]<typename C>(const C& container)
    requires std::same_as<typename C::value_type, ir::tree::Stmt>
  {
    for(auto& s : container)
    {
      std::println("{}", std::visit(ir_pretty_printer, s));
    }
  };
#endif

#if DEBUG_PRETTY_PRINT_IR
  std::println("IR");
  std::println("{}\n{}", std::visit(ir_pretty_printer, stmt), sep);
#endif

  // canonicalize the IR tree
  ir::tree::Canon canon;
  auto list = canon.linearize(std::move(stmt));

#if DEBUG_PRETTY_PRINT_CANONICALIZED_IR
  std::println("Reduced IR");
  pretty_print_stmts(list);
  std::println(sep);
#endif

  // compute the basic blocks
  auto [blocks, ldone] = canon.basic_blocks(std::move(list));

#if DEBUG_PRETTY_PRINT_BLOCKS
  std::println("Basic blocks");
  for(auto& b : blocks)
  {
    std::println("<<<< block start");
    pretty_print_stmts(b.stmts);
    std::println(">>>> block end\n");
  }
  std::println(sep);
#endif

  // lay out the blocks by following a trace
  auto sched = canon.trace_schedule(std::move(blocks), ldone);

#if DEBUG_PRETTY_PRINT_TRACE
  std::println("Trace");
  pretty_print_stmts(sched);
  std::println(sep);
#endif

  return sched;
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

#if DEBUG_PRETTY_PRINT_AST
  parser::ast::PrettyPrinter ast_printer;
  std::println("AST:\n{}", exp->accept(ast_printer));
#endif

  // find escape variables
  semant::EscapeFinder esc_finder;
  exp->accept(esc_finder);

  using Translator = ir::Translator<FrameImpl>;
  Translator translator;
  semant::Analyzer<FrameImpl> type_checker(source, string_table, translator);
  ir::Exp ir;
  try
  {
    ir = type_checker.type_check(*exp);
  }
  catch(semant::Exception& e)
  {
    terminal_write_error(e.what());
    return Error::SEMANT_ERR;
  }

  std::println("type check OK!");
  exit(0); // TODO: remove

  // AST to IR
  translator.translate_main_program(std::move(ir));
  auto out_file = fopen(out_name.c_str(), "w");
  if(!out_file)
  {
    terminal_write_error(
      std::format("tigerc: could not write assembly output for {}", source.string()));
    return Error::IO_ERR;
  }

  // IR to Assembly
  std::string assembler_directives_begin = FrameImpl::assembler_directives_begin();
  std::fwrite(assembler_directives_begin.c_str(), 1, assembler_directives_begin.size(), out_file);

  // dump fragments
  for(auto&& frag : translator.fragments())
  {
    if(std::holds_alternative<ir::ProcedureFragment<FrameImpl>>(frag))
    {
      auto& pf = std::get<ir::ProcedureFragment<FrameImpl>>(frag);
      auto stmts_list = linearize_tree(std::move(pf.body));
      emit_procedure_fragment(out_file, *pf.level->frame, stmts_list);
    }
    else if(std::holds_alternative<ir::StringFragment>(frag))
    {
      auto str = FrameImpl::emit_string(std::get<ir::StringFragment>(frag)) + "\n";
      std::fwrite(str.c_str(), 1, str.size(), out_file);
    }
  }

  std::string assembler_directives_end = FrameImpl::assembler_directives_end();
  std::fwrite(assembler_directives_end.c_str(), 1, assembler_directives_end.size(), out_file);
  fclose(out_file);
  return std::nullopt;
}