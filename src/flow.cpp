#include <algorithm>
#include <codegen/assem.hpp>
#include <flow.hpp>
#include <generated/config.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#if CONFIG_WITH_GRAPHVIZ
#  include <cstdio>
#  include <graphviz/cgraph.h>
#  include <graphviz/gvc.h>
#  include <graphviz/gvcext.h>
#  include <iostream>
#endif

namespace flow
{

FlowGraph::FlowGraph(const std::list<codegen::assem::Instruction>& ins)
{
  using node_id_t = Digraph<GraphNode>::node_id_t;
  node_id_t curr{};
  auto citer = ins.begin();
  std::optional<node_id_t> prev{};
  std::unordered_map<ir::TempGen::Label, node_id_t> label_map;

  // create nodes for labels
  for(; citer != ins.end(); citer++)
  {
    bool nid_created{};
    node_id_t nid;
    while(std::holds_alternative<::codegen::assem::Label>(*citer))
    {
      // consecutive labels point at the same node
      if(!nid_created)
      {
        nid = add_node(FlowNode());
        nid_created = true;
      }
      label_map[std::get<::codegen::assem::Label>(*citer++).label] = nid;
    }
  }

  citer = ins.begin();
  while(citer != ins.end())
  {
    if(std::holds_alternative<::codegen::assem::Label>(*citer))
    {
      curr = label_map[std::get<::codegen::assem::Label>(*citer).label];
      while(std::holds_alternative<::codegen::assem::Label>(*++citer)) // skip labels
      { }
    }
    else
    {
      // create a new node if not a label
      curr = add_node(FlowNode());
    }
    auto& i = *citer;

    if(std::holds_alternative<::codegen::assem::Oper>(i))
    {
      auto oper = std::get<::codegen::assem::Oper>(i);
      auto& data = get_node(curr).data();
      data.i = i;
      std::sort(oper.dst.begin(), oper.dst.end());
      std::sort(oper.src.begin(), oper.src.end());
      data.def = std::list(oper.dst.begin(), oper.dst.end());
      data.use = std::list(oper.src.begin(), oper.src.end());
      // add edges to jump nodes
      if(oper.jmp)
      {
        for(auto l : oper.jmp.value())
        {
          add_edge(curr, label_map[l]);
        }
      }
    }
    else if(std::holds_alternative<::codegen::assem::Move>(i))
    {
      auto move = std::get<::codegen::assem::Move>(i);
      auto& data = get_node(curr).data();
      data.is_move = true;
      data.i = i;
      data.def = {move.dst};
      data.use = {move.src};
    }
    else
    {
      assert(false);
    }

    if(prev)
    {
      // add an edge between the prev instruction and the current one
      add_edge(*prev, curr);
    }

    // update prev
    prev = curr;
    citer++;
  }
}

#if CONFIG_WITH_GRAPHVIZ
void FlowGraph::render(const std::string& name, const std::string& filename)
{
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

  std::unordered_map<Digraph<GraphNode>::node_id_t, Agnode_t*> map;
  for(auto& e : edges())
  {
    auto& n1 = get_node(e.first);
    auto& n2 = get_node(e.second);
    if(!map.contains(n1.id()))
    {
      map[n1.id()] = agnode(graph, n1.str().data(), true);
    }
    if(!map.contains(n2.id()))
    {
      map[n2.id()] = agnode(graph, n2.str().data(), true);
    }
    agedge(graph, map[n1.id()], map[n2.id()], nullptr, true);
  }

  gvLayout(gvc, graph, "dot");
  gvRender(gvc, graph, "png", outFile);
  fclose(outFile);
  gvFreeContext(gvc);
  agclose(graph);
}
#endif

} // namespace flow