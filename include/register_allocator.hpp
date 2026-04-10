#pragma once
#include "assem.hpp"
#include "flow.hpp"
#include "temp.hpp"
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

class IteratedRegisterCoalescing
{
  public:
  using color_t = std::string;
  IteratedRegisterCoalescing(
    std::shared_ptr<flow::FlowGraph> fg,
    const std::unordered_map<TempGen::Temp, color_t>& precolored_temporaries);
  IteratedRegisterCoalescing(const IteratedRegisterCoalescing&) = delete;
  auto operator=(const IteratedRegisterCoalescing&) -> IteratedRegisterCoalescing& = delete;
  IteratedRegisterCoalescing(IteratedRegisterCoalescing&&) = delete;
  auto operator=(IteratedRegisterCoalescing&&) -> IteratedRegisterCoalescing& = delete;
  ~IteratedRegisterCoalescing() = default;

  auto perform_allocation() -> const std::unordered_set<TempGen::Temp>&;
  auto get_color_mapping() -> std::function<color_t(const TempGen::Temp&)>
  {
    return [this](const TempGen::Temp& t) -> color_t {
      auto nid = map_tnode[t];
      assert(nodes[nid].color.has_value());
      return nodes[nid].color.value();
    };
  }

#if CONFIG_WITH_GRAPHVIZ
  void render_igraph_dot(const std::string& name, const std::string& filename);
#endif

  private:
  using node_id_t = size_t;
  using edge_t = std::pair<node_id_t, node_id_t>;
  struct EdgeHash
  {
    auto operator()(const edge_t& p) const noexcept -> std::size_t
    {
      return std::rotl(std::hash<node_id_t>{}(p.first), 1) ^ std::hash<node_id_t>{}(p.second);
    }
  };

  struct MoveHash
  {
    auto operator()(const assem::Move& p) const noexcept -> std::size_t
    {
      return std::rotl(std::hash<TempGen::Temp>{}(p.dst), 1) ^ std::hash<TempGen::Temp>{}(p.src);
    }
  };

  struct INode
  {
    node_id_t id;
    TempGen::Temp t;
    size_t degree{};
    std::optional<color_t> color{std::nullopt};
    std::unordered_set<node_id_t> adj{};
    std::optional<node_id_t> alias{};
    std::unordered_set<assem::Move, MoveHash> moves_list{};
  };

  std::shared_ptr<flow::FlowGraph> fgraph;
  // a mapping between temporaries and nodes in the interference graph
  std::unordered_map<TempGen::Temp, node_id_t> map_tnode;
  std::vector<INode> nodes;
  std::unordered_set<edge_t, EdgeHash> edges;

  // data structures used by the allocation algorithm
  std::list<node_id_t> simplify_list;
  std::list<node_id_t> select_stack;
  std::list<node_id_t> spill_list;
  std::unordered_set<TempGen::Temp> spilled_temps;
  std::unordered_set<node_id_t> coalesced_nodes;
  std::unordered_set<assem::Move, MoveHash> worklist_moves;
  std::unordered_set<assem::Move, MoveHash> active_moves;
  std::unordered_set<node_id_t> freeze_worklist;

  const size_t n_colors;
  const std::unordered_map<TempGen::Temp, color_t>& precolored_temporaries;

  void build_interference_graph();
  void simplify();
  void freeze();
  void select_spill();
  void make_lists();
  void assign_colors();
  void enable_moves(const std::unordered_set<node_id_t>& list);
  auto node_moves(node_id_t) -> std::unordered_set<assem::Move, MoveHash>;
  void freeze_moves(node_id_t u);
  auto get_alias(node_id_t n) const -> node_id_t;
  void coalesce();
  void combine(node_id_t u, node_id_t v);
  auto is_move_related(node_id_t n) -> bool;
  void list_push_front(std::list<node_id_t>& l, node_id_t a);
  auto is_colored(node_id_t n) -> bool
  {
    return nodes[n].color.has_value();
  }

  auto adjacent(node_id_t nid) -> std::unordered_set<node_id_t>;
  void decrement_degree(node_id_t n);
  void add_edge(node_id_t a, node_id_t b);
  auto add_node(TempGen::Temp t) -> node_id_t;
};