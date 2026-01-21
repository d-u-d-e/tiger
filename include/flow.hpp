#pragma once

#include "assem.hpp"
#include "generated/autoconf.hpp"
#include "graph.hpp"
#include "temp.hpp"
#include <functional>
#include <list>
#include <string>

namespace flow
{

struct FlowNode
{
  // the node assem instruction (could also be a basic block)
  assem::Instruction i;

  // temporaries defined at this node, sorted
  std::list<TempGen::Temp> def{};

  // temporaries used at this node, sorted
  std::list<TempGen::Temp> use{};

  // is the instruction a Move instruction?
  bool is_move{false};

  // these are used by the liveness analyzer, and are sorted by TempGen::Temp value
  std::list<TempGen::Temp> live_in{};
  std::list<TempGen::Temp> live_out{};
};

class FlowGraph : public Digraph<FlowNode>
{
  public:
  FlowGraph(const std::list<assem::Instruction>& ins,
            std::function<std::string(const TempGen::Temp& t)> temporary_mapper);

#if CONFIG_WITH_GRAPHVIZ
  void render(const std::string& name, const std::string& filename);
#endif

  private:
  std::function<std::string(const TempGen::Temp& t)> temporary_mapper;
};
} // namespace flow
