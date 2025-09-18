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
  {}
  void perform_allocation();

#ifdef CONFIG_WITH_GRAPHVIZ
  void render_igraph_dot(const std::string& name, const std::string& filename);
#endif

  struct INode {
    ir::TempGen::Temp t;
    size_t
      degree{}; // this is going to change during the simplify phase, but for effieciency we don't want to remove a node
    bool simplify_list{}; // does this node belong to the simplify list?
  };
  using node_id_t = Graph<INode>::node_id_t;

  private:
  // a mapping between temporaries and nodes in the interference graph
  std::unordered_map<ir::TempGen::Temp, node_id_t> map_tnode;
  Graph<INode> igraph;
  flow::FlowGraph& fgraph;

  std::list<node_id_t> simplify_list;
  std::list<node_id_t> select_stack;
  std::list<node_id_t> spill_list;

  void build_interference_graph();
  void simplify();
  void make_lists();
  void assign_colors();
  std::list<node_id_t> adjacent(node_id_t t);
  void decrement_degree(node_id_t n);
};

} // namespace register_allocator