#include <cassert>
#include <codegen/arch/frame.hpp>
#include <codegen/arch/isel.hpp>
#include <codegen/assem.hpp>
#include <cstddef>
#include <cstdio>
#include <flow.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#include <graphviz/gvcext.h>

namespace flow
{

FlowGraph::FlowGraph(const std::vector<codegen::assem::Instruction>& ins)
{
  size_t current_i{0};
  std::shared_ptr<Node> curr{};
  std::shared_ptr<Node> prev{};
  std::unordered_map<ir::TempGen::Label, std::shared_ptr<Node>> label_map;

  // create nodes for labels
  for(auto& i : ins)
  {
    if(std::holds_alternative<::codegen::assem::Label>(i))
    {
      auto l = std::get<::codegen::assem::Label>(i).label;
      auto n = std::make_shared<Node>();
      g.add_node(n);
      n->i = i;
      label_map[l] = n;
    }
  }

  while(current_i < ins.size())
  {
    // create a new node if not label
    auto& i = ins[current_i];
    if(!std::holds_alternative<::codegen::assem::Label>(i))
    {
      curr = std::make_shared<Node>();
      curr->i = i;
      g.add_node(curr);
    }
    else
    {
      curr = label_map[std::get<::codegen::assem::Label>(i).label];
    }

    if(prev)
    {
      // add an edge between the prev instruction and the current one
      g.add_edge(prev, curr);
    }

    if(std::holds_alternative<::codegen::assem::Oper>(i))
    {
      auto oper = std::get<::codegen::assem::Oper>(i);
      curr->def = oper.dst;
      curr->use = oper.src;
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
      curr->is_move = true;
      curr->def = {move.dst};
      curr->use = {move.src};
    }

    // update prev
    prev = curr;
    current_i++;
  }
}

void FlowGraph::render(const std::string& filename)
{
  // TODO move in graph
  Agraph_t* graph = agopen((char*)"G", Agdirected, nullptr);
  GVC_t* gvc = gvContext();
  std::unordered_map<Digraph::Node*, Agnode_t*> map;

  for(auto& node : g.get_nodes())
  {
    Agnode_t* c{};
    if(!map.contains(node.get()))
    {
      auto instr = dynamic_cast<Node&>(*node).i;
      auto descr = arch::codegen::format(arch::Frame::map_temp, instr);
      map[node.get()] = agnode(graph, descr.data(), true);
    }
    c = map[node.get()];
    for(auto& succ : g.succ(node))
    {
      if(!map.contains(succ.get()))
      {
        auto instr = dynamic_cast<Node&>(*succ).i;
        auto descr = arch::codegen::format(arch::Frame::map_temp, instr);
        map[succ.get()] = agnode(graph, descr.data(), true);
      }
      agedge(graph, c, map[succ.get()], nullptr, true);
    }
  }

  gvLayout(gvc, graph, "dot");
  FILE* outFile = fopen((filename + ".png").c_str(), "wb");
  gvRender(gvc, graph, "png", outFile);
  fclose(outFile);
  gvFreeContext(gvc);
  agclose(graph);
}

} // namespace flow