#include "codegen/arch/x86-64/frame.hpp"
#include "helpers.hpp"
#include <cmath>
#include <reg_alloc.hpp>

#if CONFIG_WITH_GRAPHVIZ
#  include <cstdio>
#  include <graphviz/cgraph.h>
#  include <iostream>
#endif

namespace register_allocator
{

void RegisterAllocator::build_interference_graph()
{
  for(auto& n : fgraph.get_nodes())
  {
    auto& d = fgraph[n].data();

    // for move instructions, add interference edges between
    // live out and def, except for the src of the move

    // for other instructions, add interference edges between
    // live out and def
    for(auto t1 : d.live_out)
    {
      if(!map_tnode.contains(t1))
      {
        map_tnode[t1] = igraph.add_node(INode{.t = t1});
      }
      for(auto t2 : d.def)
      {
        if(!map_tnode.contains(t2))
        {
          map_tnode[t2] = igraph.add_node(INode{.t = t2});
        }
        if(t1 != t2 && (!d.is_move || t1 != std::get<::codegen::assem::Move>(d.i).src))
        {
          auto n1 = map_tnode[t1];
          auto n2 = map_tnode[t2];
          igraph[n1].data().degree++;
          igraph[n2].data().degree++;
          igraph.add_edge(n1, n2);
        }
      }
    }
  }
}

std::list<RegisterAllocator::node_id_t> RegisterAllocator::adjacent(node_id_t t)
{
  return helpers::diff_sorted_lists(igraph.adj(t), select_stack);
}

void RegisterAllocator::decrement_degree(node_id_t n)
{
  auto& data = igraph[n].data();
  auto deg = data.degree;
  data.degree--;
  if(deg == arch::Frame::no_registers)
  {
    simplify_list.push_front(n);
  }
}

void RegisterAllocator::simplify()
{
  // we select a node from the simplify worklist
  assert(simplify_list.size() > 0);
  auto t = simplify_list.front();
  simplify_list.pop_front();
  select_stack.push_front(t);
  for(auto m : adjacent(t))
  {
    decrement_degree(m);
  }
}

void RegisterAllocator::assign_colors()
{
  // TODO
}

void RegisterAllocator::make_lists()
{
  for(auto n : igraph.get_nodes())
  {
    if(igraph[n].data().degree < arch::Frame::no_registers)
    {
      simplify_list.push_front(n);
    }
  }
}

void RegisterAllocator::perform_allocation()
{
  build_interference_graph();
  make_lists();
  while(!simplify_list.empty())
  {
    simplify();
  }
  assign_colors();
}

#ifdef CONFIG_WITH_GRAPHVIZ
void RegisterAllocator::render_igraph_dot(const std::string& name, const std::string& filename)
{
  Agraph_t* graph = agopen(const_cast<char*>(name.data()), Agstrictundirected, nullptr);

  std::string fname_ext = filename + ".txt";
  std::string err_msg = "could not render interference graph " + fname_ext;
  FILE* outFile = fopen(fname_ext.c_str(), "wb");

  if(!graph || !outFile)
  {
    std::cerr << "\033[1;31m";
    std::cerr << err_msg << std::endl;
    std::cerr << "\033[0m";
    return;
  }

  std::unordered_map<Graph<ir::TempGen::Temp>::node_id_t, Agnode_t*> map;

  for(auto& e : igraph.edges())
  {
    auto& n1 = igraph.get_node(e.first);
    auto& n2 = igraph.get_node(e.second);
    if(!map.contains(n1.id()))
    {
      std::string descr = helpers::map_temp(n1.data().t);
      map[n1.id()] = agnode(graph, descr.data(), true);
    }
    if(!map.contains(n2.id()))
    {
      std::string descr = helpers::map_temp(n2.data().t);
      map[n2.id()] = agnode(graph, descr.data(), true);
    }
    agedge(graph, map[n1.id()], map[n2.id()], nullptr, true);
  }

  agwrite(graph, outFile);
  fclose(outFile);
  agclose(graph);
}
#endif

} // namespace register_allocator