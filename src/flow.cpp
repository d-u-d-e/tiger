#include <codegen/assem.hpp>
#include <cstddef>
#include <flow.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <config.hpp>

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
  using node_id_t = Digraph<Node>::node_id_t;
  size_t current_i{};
  node_id_t curr{};
  std::optional<node_id_t> prev{};
  std::unordered_map<ir::TempGen::Label, node_id_t> label_map;

  // create nodes for labels
  for(auto& i : ins)
  {
    if(std::holds_alternative<::codegen::assem::Label>(i))
    {
      auto l = std::get<::codegen::assem::Label>(i).label;
      label_map[l] = g.add_node(Node{.i = i});
    }
  }

  while(current_i < ins.size())
  {
    // create a new node if not label
    auto& i = ins[current_i];
    if(!std::holds_alternative<::codegen::assem::Label>(i))
    {
      curr = g.add_node(Node{.i = i});
    }
    else
    {
      curr = label_map[std::get<::codegen::assem::Label>(i).label];
    }

    if(prev)
    {
      // add an edge between the prev instruction and the current one
      g.add_edge(*prev, curr);
    }

    if(std::holds_alternative<::codegen::assem::Oper>(i))
    {
      auto oper = std::get<::codegen::assem::Oper>(i);
      auto& data = g[curr].data();
      data.def = std::move(oper.dst);
      data.use = std::move(oper.src);
      // add edges to jump nodes
      if(oper.jmp)
      {
        for(auto l : oper.jmp.value())
        {
          g.add_edge(curr, label_map[l]);
        }
      }
    }
    else if(std::holds_alternative<::codegen::assem::Move>(i))
    {
      auto move = std::get<::codegen::assem::Move>(i);
      auto& data = g[curr].data();
      data.is_move = true;
      data.def = {move.dst};
      data.use = {move.src};
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

  std::unordered_map<Digraph<Node>::node_id_t, Agnode_t*> map;
  for(auto& node : g.get_nodes())
  {
    Agnode_t* c{};
    if(!map.contains(node.id()))
    {
      map[node.id()] = agnode(graph, node.str().data(), true);
    }
    c = map[node.id()];
    for(auto& succ : g.succ(node.id()))
    {
      if(!map.contains(succ))
      {
        map[succ] = agnode(graph, g[succ].str().data(), true);
      }
      agedge(graph, c, map[succ], nullptr, true);
    }
  }

  gvLayout(gvc, graph, "dot");
  gvRender(gvc, graph, "png", outFile);
  fclose(outFile);
  gvFreeContext(gvc);
  agclose(graph);
}
#endif

} // namespace flow