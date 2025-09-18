#include <algorithm>
#include <codegen/assem.hpp>
#include <cstddef>
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

FlowGraph::FlowGraph(const std::vector<codegen::assem::Instruction>& ins)
{
  using node_id_t = Digraph<GraphNode>::node_id_t;
  size_t current_i{};
  node_id_t curr{};
  std::optional<node_id_t> prev{};
  std::unordered_map<ir::TempGen::Label, node_id_t> label_map;

  // create nodes for labels
  for(; current_i < ins.size(); current_i++)
  {
    auto& i = ins[current_i];
    if(std::holds_alternative<::codegen::assem::Label>(i))
    {
      // next instruction must exist and must not be a label
      assert(current_i + 1 < ins.size());
      assert(!std::holds_alternative<::codegen::assem::Label>(ins[current_i + 1]));
      auto l = std::get<::codegen::assem::Label>(i).label;
      label_map[l] = add_node(Node());
    }
  }

  current_i = 0;
  while(current_i < ins.size())
  {
    if(std::holds_alternative<::codegen::assem::Label>(ins[current_i]))
    {
      // skip the label
      curr = label_map[std::get<::codegen::assem::Label>(ins[current_i]).label];
      current_i++;
    }
    else
    {
      // create a new node if not a label
      curr = add_node(Node());
    }
    auto& i = ins[current_i];

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
    current_i++;
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