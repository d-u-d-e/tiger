#pragma once
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <format>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace utils
{

/* A directed graph */
template <typename T>
class Digraph
{
  public:
  using node_id_t = size_t;
  using edge_t = std::pair<node_id_t, node_id_t>;
  struct GraphNode
  {
    GraphNode(node_id_t uid, T data)
      : data_(std::move(data))
      , uid(uid)
    { }

    auto str() const -> std::string requires requires(const T& t)
    {
      {
        t.to_string()
        } -> std::same_as<std::string>;
    }
    {
      return std::format("{}: {}", uid, data_.to_string());
    }

    auto str() const -> std::string
    {
      return std::to_string(uid);
    }

    auto data() -> T&
    {
      return data_;
    }

    auto id() const -> node_id_t
    {
      return uid;
    }
    friend Digraph;

private:
    T data_;
    std::unordered_set<node_id_t> prec;
    std::unordered_set<node_id_t> succ;
    node_id_t uid{};
  };

  void add_edge(node_id_t from, node_id_t to)
  {
    assert(is_valid(from) && is_valid(to) && from != to);
    nodes[from].succ.insert(to);
    nodes[to].prec.insert(from);
  }

  template <typename U = T>
  auto add_node(U&& node_data) -> node_id_t
  {
    node_id_t node_id = nodes.size();
    nodes.emplace_back(node_id, std::forward<U>(node_data));
    return node_id;
  }

  [[nodiscard]] auto succ(node_id_t nid) const -> const std::unordered_set<node_id_t>&
  {
    assert(is_valid(nid));
    return nodes[nid].succ;
  }

  [[nodiscard]] auto prec(node_id_t nid) const -> const std::unordered_set<node_id_t>&
  {
    assert(is_valid(nid));
    return nodes[nid].prec;
  }

  auto operator[](node_id_t nid) -> GraphNode&
  {
    return get_node(nid);
  }

  auto get_node(node_id_t nid) -> GraphNode&
  {
    assert(is_valid(nid));
    return nodes[nid];
  }

  [[nodiscard]] auto get_nodes() const -> std::vector<node_id_t>
  {
    std::vector<node_id_t> out(nodes.size());
    std::ranges::transform(nodes, out.begin(), [](const GraphNode& n) { return n.uid; });
    return out;
  }

  [[nodiscard]] auto edges() const -> std::vector<edge_t>
  {
    std::vector<edge_t> out;
    for(auto& n : nodes)
    {
      for(auto& a : n.succ)
      {
        out.emplace_back(n.id(), a);
      }
    }
    return out;
  }

  private:
  [[nodiscard]] auto is_valid(node_id_t nid) const -> bool
  {
    return nid < nodes.size();
  }
  std::vector<GraphNode> nodes;
};

} // namespace utils