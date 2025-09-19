#include <codegen/arch.hpp>
#include <reg_alloc.hpp>

#if CONFIG_WITH_GRAPHVIZ
#  include <cstdio>
#  include <graphviz/cgraph.h>
#  include <iostream>
#endif

namespace register_allocator
{

RegisterAllocator::RegisterAllocator() { }

void RegisterAllocator::build_interference_graph()
{
  map_tnode.clear();
  nodes.clear();
  edges.clear();

  // fill precolored temporaries
  for(auto& [t, v] : arch::Frame::temp_map)
  {
    auto nid = add_node(t);
    precolored_nodes.insert(nid);
    map_tnode[t] = nid;
    nodes[nid].color = v;
  }

  // parse the flow graph
  for(auto& n : fgraph->get_nodes())
  {
    auto& d = fgraph->get_node(n).data();

    // for move instructions, add interference edges between
    // live out and def, except for the src of the move

    // for other instructions, add interference edges between
    // live out and def
    for(auto t1 : d.live_out)
    {
      if(!map_tnode.contains(t1))
      {
        map_tnode[t1] = add_node(t1);
      }
      for(auto t2 : d.def)
      {
        if(!map_tnode.contains(t2))
        {
          map_tnode[t2] = add_node(t2);
        }
        if(t1 != t2 && (!d.is_move || t1 != std::get<::codegen::assem::Move>(d.i).src))
        {
          auto n1 = map_tnode[t1];
          auto n2 = map_tnode[t2];
          add_edge(n1, n2);
        }
      }
    }
  }
}

std::unordered_set<RegisterAllocator::node_id_t> RegisterAllocator::adjacent(node_id_t t)
{
  std::unordered_set<node_id_t> out;
  std::copy_if(
    nodes[t].adj.begin(), nodes[t].adj.end(), std::inserter(out, out.begin()), [this](node_id_t n) {
      return select_stack.end() == std::find(select_stack.begin(), select_stack.end(), n);
    });
  return out;
}

void RegisterAllocator::decrement_degree(node_id_t n)
{
  auto& data = nodes[n];
  auto deg = data.degree;
  data.degree--;
  if(deg == arch::Frame::no_registers)
  {
    simplify_list.push_front(n);
  }
}

void RegisterAllocator::add_edge(node_id_t a, node_id_t b)
{
  assert(a != b);
  if(a > b)
  {
    std::swap(a, b);
  }
  if(!edges.contains({a, b}))
  {
    edges.insert({a, b});
    if(!is_precolored(a))
    {
      nodes[a].adj.insert(b);
      nodes[a].degree++;
    }
    if(!is_precolored(b))
    {
      nodes[b].adj.insert(a);
      nodes[b].degree++;
    }
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

RegisterAllocator::node_id_t RegisterAllocator::add_node(ir::TempGen::Temp t)
{
  nodes.push_back(INode{.id = nodes.size(), .t = t});
  return nodes.size() - 1;
}

void RegisterAllocator::assign_colors()
{
  while(!select_stack.empty())
  {
    auto top = select_stack.front();
    select_stack.pop_front();
    std::unordered_set<arch::Frame::register_t> ok_colors{colors};
    for(auto n : nodes[top].adj)
    {
      if(is_precolored(n) || colored_nodes.contains(n))
      {
        assert(nodes[n].color.has_value());
        ok_colors.erase(nodes[n].color.value());
      }
      if(ok_colors.empty())
      {
        spill_list.push_front(top);
      }
      else
      {
        colored_nodes.insert(top);
        nodes[top].color = *ok_colors.begin();
      }
    }
  }
}

void RegisterAllocator::make_lists()
{
  initial_nodes.clear();
  colored_nodes.clear();
  simplify_list.clear();
  select_stack.clear();
  spill_list.clear();

  for(auto& n : nodes)
  {
    if(!is_precolored(n.id))
    {
      initial_nodes.insert(n.id);
    }
  }

  for(auto& nid : initial_nodes)
  {
    if(nodes[nid].degree >= arch::Frame::no_registers)
    {
      spill_list.push_front(nid);
    }
    else
    {
      simplify_list.push_front(nid);
    }
  }
}

void RegisterAllocator::perform_allocation()
{
  assert(fgraph != nullptr);
  build_interference_graph();
  make_lists();
  while(!simplify_list.empty())
  {
    simplify();
  }
  assign_colors();
  if(!spill_list.empty())
  {
    // TODO: implement spilling
    std::cerr << "\033[1;31m";
    std::cerr << "spilling not implemented yet, aborting\n";
    std::cerr << "\033[0m";
    exit(1);
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

  for(auto& e : edges)
  {
    auto& n1 = e.first;
    auto& n2 = e.second;
    if(!map.contains(n1))
    {
      std::string descr = helpers::map_temp(map_tnode[n1]);
      map[n1] = agnode(graph, descr.data(), true);
    }
    if(!map.contains(n2))
    {
      std::string descr = helpers::map_temp(map_tnode[n2]);
      map[n2] = agnode(graph, descr.data(), true);
    }
    agedge(graph, map[n1], map[n2], nullptr, true);
  }

  agwrite(graph, outFile);
  fclose(outFile);
  agclose(graph);
}
#endif

} // namespace register_allocator