#include <cstdio>
#include <graph.hpp>
#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <graphviz/gvcext.h>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>

void Digraph::render(const std::string& name, const std::string& filename)
{
  // TODO move in graph
  Agraph_t* graph = agopen(const_cast<char*>(name.data()), Agdirected, nullptr);
  GVC_t* gvc = gvContext();

  std::string fname_ext = filename + ".png";
  std::string err_msg = "could not render flow graph " + fname_ext;
  FILE* outFile = fopen(fname_ext.c_str(), "wb");

  if(!graph || !gvc || !outFile)
  {
    std::cerr << "\033[1;31m";
    std::cerr << err_msg << std::endl;
    std::cerr << "\033[0m";
    return;
  }

  std::unordered_map<Digraph::Node*, Agnode_t*> map;
  for(auto& node : nodes)
  {
    Agnode_t* c{};
    if(!map.contains(node.get()))
    {
      map[node.get()] = agnode(graph, node->str().data(), true);
    }
    c = map[node.get()];
    for(auto& succ : succ(node))
    {
      if(!map.contains(succ.get()))
      {
        map[succ.get()] = agnode(graph, succ->str().data(), true);
      }
      agedge(graph, c, map[succ.get()], nullptr, true);
    }
  }

  gvLayout(gvc, graph, "dot");
  gvRender(gvc, graph, "png", outFile);
  fclose(outFile);
  gvFreeContext(gvc);
  agclose(graph);
}