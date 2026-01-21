#include "flow.hpp"
#include "assem.hpp"

#if CONFIG_WITH_GRAPHVIZ
#  include <cstdio>
#  include <graphviz/cgraph.h>
#  include <graphviz/gvc.h>
#  include <graphviz/gvcext.h>
#  include <iostream>
#endif

namespace flow
{

FlowGraph::FlowGraph(const std::list<assem::Instruction>& ins,
                     std::function<std::string(const TempGen::Temp& t)> temporary_mapper)
  : temporary_mapper(temporary_mapper)
{
  using node_id_t = Digraph<GraphNode>::node_id_t;
  node_id_t curr{};
  std::optional<node_id_t> prev{};
  std::unordered_map<TempGen::Label, node_id_t> label_map;

  // create nodes for labels
  for(auto& i : ins)
  {
    if(std::holds_alternative<assem::Label>(i))
    {
      auto nid = add_node(FlowNode{.i = i});
      label_map[std::get<assem::Label>(i).label] = nid;
    }
  }

  for(auto& i : ins)
  {
    if(std::holds_alternative<assem::Label>(i))
    {
      curr = label_map[std::get<assem::Label>(i).label];
    }
    else
    {
      // create a new node if not a label
      curr = add_node(FlowNode{.i = i});
    }

    if(prev)
    {
      // add an edge between the prev instruction and the current one
      add_edge(*prev, curr);
    }

    bool curr_is_jmp{};
    if(std::holds_alternative<assem::Oper>(i))
    {
      auto oper = std::get<assem::Oper>(i);
      auto& data = get_node(curr).data();
      std::sort(oper.dst.begin(), oper.dst.end());
      std::sort(oper.src.begin(), oper.src.end());
      data.def = std::list(oper.dst.begin(), oper.dst.end());
      data.use = std::list(oper.src.begin(), oper.src.end());
      // add edges to jump nodes
      if(oper.jmp)
      {
        curr_is_jmp = true;
        for(auto l : oper.jmp.value())
        {
          add_edge(curr, label_map[l]);
        }
      }
    }
    else if(std::holds_alternative<assem::Move>(i))
    {
      auto move = std::get<assem::Move>(i);
      auto& data = get_node(curr).data();
      data.is_move = true;
      data.def = {move.dst};
      data.use = {move.src};
    }
    prev = curr_is_jmp ? std::nullopt : std::optional(curr);
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
      auto descr = std::format("{}: {}", n1.id(), assem::format(temporary_mapper, n1.data().i));
      map[n1.id()] = agnode(graph, descr.data(), true);
    }
    if(!map.contains(n2.id()))
    {
      auto descr = std::format("{}: {}", n2.id(), assem::format(temporary_mapper, n2.data().i));
      map[n2.id()] = agnode(graph, descr.data(), true);
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