#pragma once
#include <cassert>
#include <concepts>
#include <cstddef>
#include <format>
#include <graphviz/types.h>
#include <list>
#include <string>
#include <unordered_set>
#include <vector>

/* A directed graph */
template <typename T>
class Digraph {
  public:
  using node_id_t = size_t;
  using edge_t = std::pair<node_id_t, node_id_t>;
  struct GraphNode {
    GraphNode(node_id_t uid, T data)
      : data_(std::move(data))
      , uid(uid)
    { }

    std::string str() const
      requires requires(const T& t) {
        { t.to_string() } -> std::same_as<std::string>;
      }
    {
      return std::format("{}: {}", uid, data_.to_string());
    }

    std::string str() const
    {
      return std::to_string(uid);
    }

    T& data()
    {
      return data_;
    }

    node_id_t id() const
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

  node_id_t add_node(T node_data)
  {
    node_id_t node_id = nodes.size();
    nodes.emplace_back(node_id, std::move(node_data));
    return node_id;
  }

  const std::unordered_set<node_id_t>& succ(node_id_t nid) const
  {
    assert(is_valid(nid));
    return nodes[nid].succ;
  }

  const std::unordered_set<node_id_t>& prec(node_id_t nid) const
  {
    assert(is_valid(nid));
    return nodes[nid].prec;
  }

  GraphNode& operator[](node_id_t nid)
  {
    return get_node(nid);
  }

  GraphNode& get_node(node_id_t nid)
  {
    assert(is_valid(nid));
    return nodes[nid];
  }

  std::list<node_id_t> get_nodes()
  {
    std::list<node_id_t> out;
    for(auto& n : nodes)
    {
      out.push_back(n.id());
    }
    return out;
  }

  std::vector<edge_t> edges() const
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
  bool is_valid(node_id_t nid) const
  {
    return nid < nodes.size();
  }
  std::vector<GraphNode> nodes;
};

/* A standard undirected graph */
template <typename T>
class Graph {
  public:
  using node_id_t = size_t;
  using edge_t = std::pair<node_id_t, node_id_t>;

  struct EdgeHash {
    std::size_t operator()(const edge_t& p) const noexcept
    {
      // Simple combination of x and y
      return std::rotl(std::hash<node_id_t>{}(p.first), 1) ^ std::hash<node_id_t>{}(p.second);
    }
  };

  struct GraphNode {
    GraphNode(node_id_t uid, T data)
      : data_(std::move(data))
      , uid(uid)
    { }

    std::string str() const
      requires requires(const T& t) {
        { t.to_string() } -> std::same_as<std::string>;
      }
    {
      return std::format("{}: {}", uid, data_.to_string());
    }

    std::string str() const
    {
      return std::to_string(uid);
    }

    T& data()
    {
      return data_;
    }

    node_id_t id() const
    {
      return uid;
    }
    friend Graph;

private:
    T data_;
    std::list<node_id_t> adj;
    node_id_t uid{};
  };

  void add_edge(node_id_t a, node_id_t b)
  {
    assert(is_valid(a) && is_valid(b) && a != b);
    if(a > b)
    {
      std::swap(a, b);
    }
    if(edges_.contains({a, b}))
    {
      return;
    }
    edges_.insert({a, b});
    nodes[a].adj.push_front(b);
    nodes[b].adj.push_front(a);
  }

  node_id_t add_node(T node_data)
  {
    auto node_id = nodes.size();
    nodes.emplace_back(node_id, std::move(node_data));
    return node_id;
  }

  const std::list<node_id_t>& adj(node_id_t nid) const
  {
    assert(is_valid(nid));
    return nodes[nid].adj;
  }

  GraphNode& operator[](node_id_t nid)
  {
    return get_node(nid);
  }

  GraphNode& get_node(node_id_t nid)
  {
    assert(is_valid(nid));
    return nodes[nid];
  }

  std::list<node_id_t> get_nodes()
  {
    std::list<node_id_t> out;
    for(auto& n : nodes)
    {
      out.push_back(n.id());
    }
    return out;
  }

  auto& edges() const
  {
    return edges_;
  }

  private:
  bool is_valid(node_id_t nid) const
  {
    return nid < nodes.size() && !free_nodes_ids.contains(nid);
  }
  std::unordered_set<edge_t, EdgeHash> edges_;
  std::unordered_set<node_id_t> free_nodes_ids;
  std::vector<GraphNode> nodes;
};
