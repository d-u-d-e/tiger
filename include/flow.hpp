#pragma once

#include <codegen/assem.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <string>
#include <vector>

namespace flow
{
class FlowGraph {
  public:
  struct Node : public Digraph::Node {
    // the node assem instruction (could also be a basic block)
    codegen::assem::Instruction i;

    // temporaries defined at this node
    std::vector<ir::TempGen::Temp> def;

    // temporaries used at this node
    std::vector<ir::TempGen::Temp> use;

    // is the instruction a Move instruction?
    bool is_move{false};
  };

  FlowGraph(const std::vector<::codegen::assem::Instruction>& ins);
  void render(const std::string& filename);

  private:
  Digraph g;
};
} // namespace flow
