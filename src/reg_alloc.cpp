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
    auto& d = n.data();

    // for move instructions, add interference edges between
    // live out and def, except for the src of the move

    // for other instructions, add interference edges between
    // live out and def
    for(auto t1 : d.live_out)
    {
      if(!map_tnode.contains(t1))
      {
        map_tnode[t1] = igraph.add_node(t1);
      }
      for(auto t2 : d.def)
      {
        if(!map_tnode.contains(t2))
        {
          map_tnode[t2] = igraph.add_node(t2);
        }
        if(t1 != t2 && (!d.is_move || t1 != std::get<::codegen::assem::Move>(d.i).src))
        {
          igraph.add_edge(map_tnode[t1], map_tnode[t2]);
        }
      }
    }
  }
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
      std::string descr = helpers::map_temp(n1.data());
      map[n1.id()] = agnode(graph, descr.data(), true);
    }
    if(!map.contains(n2.id()))
    {
      std::string descr = helpers::map_temp(n2.data());
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