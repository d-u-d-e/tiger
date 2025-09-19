#pragma once

#include <helpers.hpp>
#include <codegen/arch.hpp>
#include <codegen/assem.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <list>
#include <string>
#include <vector>

namespace flow
{

struct FlowNode {
  // the node assem instruction (could also be a basic block)
  codegen::assem::Instruction i;

  // temporaries defined at this node, sorted
  std::list<ir::TempGen::Temp> def{};

  // temporaries used at this node, sorted
  std::list<ir::TempGen::Temp> use{};

  // is the instruction a Move instruction?
  bool is_move{false};

  // these are used by the liveness analyzer, and are sorted by ir::TempGen::Temp value
  std::list<ir::TempGen::Temp> live_in{};
  std::list<ir::TempGen::Temp> live_out{};

  std::string to_string() const
  {
    return arch::codegen::format(helpers::map_temp, i);
  }
};

class FlowGraph : public Digraph<FlowNode> {
  public:
  FlowGraph(const std::list<::codegen::assem::Instruction>& ins);

#if CONFIG_WITH_GRAPHVIZ
  void render(const std::string& name, const std::string& filename);
#endif
};
} // namespace flow
