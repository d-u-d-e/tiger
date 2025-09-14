#pragma once

#include <codegen/arch/frame.hpp>
#include <codegen/arch/isel.hpp>
#include <codegen/assem.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <list>
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

    // these are used by the liveness analyzer, and are sorted by ir::TempGen::Temp value
    std::list<ir::TempGen::Temp> live_in;
    std::list<ir::TempGen::Temp> live_out;

    std::string str() const override
    {
      return arch::codegen::format(arch::Frame::map_temp, i);
    }
  };

  FlowGraph(const std::vector<::codegen::assem::Instruction>& ins);

  void render(const std::string& name, const std::string& filename)
  {
    g.render(name, filename);
  }

  private:
  Digraph g;
};
} // namespace flow
