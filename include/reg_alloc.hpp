#pragma once
#include <flow.hpp>
#include <generated/config.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>

namespace register_allocator
{

class RegisterAllocator {
  public:
  RegisterAllocator(flow::FlowGraph& fg)
    : fgraph(fg)
  { }
  void build_interference_graph();

#ifdef CONFIG_WITH_GRAPHVIZ
  void render_igraph_dot(const std::string& name, const std::string& filename);
#endif

  private:
  // a mapping between temporaries and nodes in the interference graph
  std::unordered_map<ir::TempGen::Temp, Graph<ir::TempGen::Temp>::node_id_t> map_tnode;
  Graph<ir::TempGen::Temp> igraph;
  flow::FlowGraph& fgraph;
};

} // namespace register_allocator