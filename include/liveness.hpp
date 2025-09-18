#pragma once
#include <flow.hpp>
#include <ir/temp.hpp>
#include <unordered_map>

namespace liveness
{
class LivenessAnalyzer {
  public:
  LivenessAnalyzer(flow::FlowGraph& g);
  std::string dump_result();
  const Graph<ir::TempGen::Temp>& interference_graph() const
  {
    return igraph;
  }

#ifdef CONFIG_WITH_GRAPHVIZ
  void render_igraph_dot(const std::string& name, const std::string& filename);
#endif

  private:
  void make_interference_graph();
  // a mapping between temporaries and nodes in the interference graph
  std::unordered_map<ir::TempGen::Temp, Graph<ir::TempGen::Temp>::node_id_t> map_tnode;
  // the inverse mapping is already available in the graph nodes
  Graph<ir::TempGen::Temp> igraph; // the interference graph
  flow::FlowGraph& fg;
};
} // namespace liveness