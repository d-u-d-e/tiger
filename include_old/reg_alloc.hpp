#pragma once
#include <codegen/arch/x86-64/frame.hpp>
#include <codegen/assem.hpp>
#include <cstddef>
#include <flow.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <optional>
#include <ranges>
#include <sys/types.h>
#include <unordered_set>

namespace register_allocator
{

class IteratedRegisterCoalescing {
  public:
  static inline size_t K = arch::Frame::no_registers;

  IteratedRegisterCoalescing(std::shared_ptr<flow::FlowGraph> fg);
  const std::unordered_set<ir::TempGen::Temp>& perform_allocation();
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

  struct MoveHash {
    std::size_t operator()(const ::codegen::assem::Move& p) const noexcept
    {
      return std::rotl(std::hash<ir::TempGen::Temp>{}(p.dst), 1) ^
             std::hash<ir::TempGen::Temp>{}(p.src);
    }
  };

  struct INode {
    node_id_t id;
    ir::TempGen::Temp t;
    size_t degree{};
    std::optional<arch::Frame::register_t> color{};
    std::unordered_set<node_id_t> adj{};
    std::optional<node_id_t> alias{};
    std::unordered_set<::codegen::assem::Move, MoveHash> moves_list{};

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

  // data structures used by the allocation algorithm
  std::list<node_id_t> simplify_list;
  std::list<node_id_t> select_stack;
  std::list<node_id_t> spill_list;
  std::unordered_set<ir::TempGen::Temp> spilled_temps;
  std::unordered_set<node_id_t> coalesced_nodes;
  std::unordered_set<::codegen::assem::Move, MoveHash> worklist_moves;
  std::unordered_set<::codegen::assem::Move, MoveHash> active_moves;
  std::unordered_set<node_id_t> freeze_worklist;

  static inline auto colors =
    std::ranges::to<std::unordered_set>(std::ranges::views::values(arch::Frame::temp_map));

  void build_interference_graph();
  void simplify();
  void freeze();
  void select_spill();
  void make_lists();
  void assign_colors();
  void enable_moves(const std::unordered_set<node_id_t>& nodes);
  std::unordered_set<::codegen::assem::Move, MoveHash> node_moves(node_id_t);
  void freeze_moves(node_id_t n);
  node_id_t get_alias(node_id_t n);
  void coalesce();
  void combine(node_id_t u, node_id_t v);
  bool is_move_related(node_id_t n);
  void list_push_front(std::list<node_id_t>& l, node_id_t a);
  bool is_colored(node_id_t n)
  {
    return nodes[n].color.has_value();
  }

  std::unordered_set<node_id_t> adjacent(node_id_t t);
  void decrement_degree(node_id_t n);
  void add_edge(node_id_t a, node_id_t b);
  node_id_t add_node(ir::TempGen::Temp t);
};

} // namespace register_allocator