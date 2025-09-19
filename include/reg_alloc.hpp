#pragma once
#include <flow.hpp>
#include <generated/config.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <ranges>

namespace register_allocator
{

class RegisterAllocator {
  public:
  RegisterAllocator(std::shared_ptr<flow::FlowGraph> fg);
  void perform_allocation();
  std::function<arch::Frame::register_t(const ir::TempGen::Temp&)> get_color_mapping()
  {
    return [this](const ir::TempGen::Temp& t) -> arch::Frame::register_t {
      auto nid = map_tnode[t];
      assert(nodes[nid].color.has_value());
      return nodes[nid].color.value();
    };
  }

#ifdef CONFIG_WITH_GRAPHVIZ
  void render_igraph_dot(const std::string& name, const std::string& filename);
#endif

  using node_id_t = size_t;
  using edge_t = std::pair<node_id_t, node_id_t>;

  struct EdgeHash {
    std::size_t operator()(const edge_t& p) const noexcept
    {
      return std::rotl(std::hash<node_id_t>{}(p.first), 1) ^ std::hash<node_id_t>{}(p.second);
    }
  };

  struct INode {
    node_id_t id;
    ir::TempGen::Temp t;
    size_t degree{};
    std::optional<arch::Frame::register_t> color{};
    std::unordered_set<node_id_t> adj{};

    std::string to_string() const
    {
      return std::format("{}", helpers::map_temp(t));
    }
  };

  private:
  std::shared_ptr<flow::FlowGraph> fgraph;
  // a mapping between temporaries and nodes in the interference graph
  std::unordered_map<ir::TempGen::Temp, node_id_t> map_tnode;
  std::vector<INode> nodes;
  std::unordered_set<edge_t, EdgeHash> edges;

  // lists used during the allocation algorithm
  std::list<node_id_t> simplify_list;
  std::list<node_id_t> select_stack;
  std::list<node_id_t> spill_list;

  // sets used during the allocation algorithm
  std::unordered_set<node_id_t> colored_nodes;
  std::unordered_set<node_id_t> precolored_nodes;
  std::unordered_set<node_id_t> initial_nodes;

  static inline auto colors =
    std::ranges::to<std::unordered_set>(std::ranges::views::values(arch::Frame::temp_map));

  void build_interference_graph();
  void simplify();
  void make_lists();
  void assign_colors();
  bool is_precolored(node_id_t n)
  {
    return precolored_nodes.contains(n);
  }

  std::unordered_set<node_id_t> adjacent(node_id_t t);
  void decrement_degree(node_id_t n);
  void add_edge(node_id_t a, node_id_t b);
  node_id_t add_node(ir::TempGen::Temp t);
};

} // namespace register_allocator