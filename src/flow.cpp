#include <codegen/assem.hpp>
#include <cstddef>
#include <flow.hpp>
#include <graph.hpp>
#include <ir/temp.hpp>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

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

} // namespace flow