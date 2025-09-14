#pragma once
#include <cassert>
#include <cstddef>
#include <list>
#include <memory>
#include <string>
#include <unordered_set>

class Digraph {
  public:
  using node_id_t = size_t;
  struct Node {
    Node()
      : uid(uid_counter++)
    { }
    virtual std::string id() const
    {
      return std::to_string(uid);
    }
    friend Digraph;

private:
    static inline node_id_t uid_counter{};
    std::unordered_set<std::shared_ptr<Node>> prec;
    std::unordered_set<std::shared_ptr<Node>> succ;
    node_id_t uid{};
  };

  Digraph() { }

  void add_edge(const std::shared_ptr<Node>& from, const std::shared_ptr<Node>& to)
  {
    from->succ.insert(to);
    to->prec.insert(from);
  }

  void remove_edge(const std::shared_ptr<Node>& from, const std::shared_ptr<Node>& to)
  {
    from->succ.erase(to);
    to->prec.erase(from);
  }

  void add_node(std::shared_ptr<Node> node)
  {
    nodes.push_back(node);
  }

  void delete_node(const std::shared_ptr<Node>& n)
  {
    (void)n;
    assert(false);
    //TODO
  }

  const std::unordered_set<std::shared_ptr<Node>>& succ(const std::shared_ptr<Node>& n)
  {
    return n->succ;
  }

  const std::unordered_set<std::shared_ptr<Node>>& prec(const std::shared_ptr<Node>& n)
  {
    return n->prec;
  }

  const std::list<std::shared_ptr<Node>>& get_nodes()
  {
    return nodes;
  }

  private:
  std::list<std::shared_ptr<Node>> nodes;
};
