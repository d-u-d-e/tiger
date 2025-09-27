#include "codegen/arch/x86-64/frame.hpp"
#include "helpers.hpp"
#include "ir/temp.hpp"
#include <algorithm>
#include <cassert>
#include <codegen/arch.hpp>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <graphviz/types.h>
#include <limits>
#include <reg_alloc.hpp>
#include <unordered_set>
#include <vector>

#if CONFIG_WITH_GRAPHVIZ
#  include <cstdio>
#  include <graphviz/cgraph.h>
#  include <iostream>
#endif

namespace register_allocator
{

RegisterAllocator::RegisterAllocator(std::shared_ptr<flow::FlowGraph> fg)
  : fgraph(std::move(fg))
{
  // fill precolored temporaries
  for(auto& [t, v] : arch::Frame::temp_map)
  {
    auto nid = add_node(t);
    map_tnode[t] = nid;
    nodes[nid].color = v;
    // precolored temporaries have infinite degree and thus cannot be simplified
    nodes[nid].degree = std::numeric_limits<std::size_t>::max();
  }

  build_interference_graph();

  // add move instructions to the moves list of involved nodes
  for(auto& n : fgraph->get_nodes())
  {
    auto& d = fgraph->get_node(n).data();
    if(d.is_move)
    {
      auto& i = std::get<::codegen::assem::Move>(d.i);
      nodes[map_tnode[i.src]].moves_list.insert(i);
      nodes[map_tnode[i.dst]].moves_list.insert(i);
      worklist_moves.insert(i);
    }
  }
}

void RegisterAllocator::build_interference_graph()
{
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
          if(n1 == 23 || n2 == 23)
          {
            auto x = 2;
            (void)x;
          }
        }
      }
    }
  }
}

void RegisterAllocator::list_push_front(std::list<node_id_t>& l, node_id_t a)
{
  if(l == simplify_list)
  {
    assert(!is_colored(a));
  }
  assert(std::find(l.begin(), l.end(), a) == l.end());
  l.push_front(a);
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
    if(!is_colored(a))
    {
      std::cout << std::format("inc deg of {}:{}\n", a, helpers::map_temp(nodes[a].t));
      nodes[a].adj.insert(b);
      nodes[a].degree++;
    }
    if(!is_colored(b))
    {
      std::cout << std::format("inc deg of {}:{}\n", b, helpers::map_temp(nodes[b].t));
      nodes[b].adj.insert(a);
      nodes[b].degree++;
    }
  }
}

void RegisterAllocator::make_lists()
{
  for(auto& u : nodes)
  {
    auto nid = u.id;
    // precolored nodes don't belong to any list
    if(is_colored(nid))
    {
      continue;
    }
    else if(u.degree >= K)
    {
      // high degree nodes are put into the spill list
      list_push_front(spill_list, nid);
    }
    else if(is_move_related(nid))
    {
      // low degree move-related nodes are put into the freeze list
      list_push_front(freeze_worklist, nid);
    }
    else
    {
      // low degree non move-related nodes are put into the simplify list
      list_push_front(simplify_list, nid);
    }
  }
}

std::unordered_set<RegisterAllocator::node_id_t> RegisterAllocator::adjacent(node_id_t nid)
{
  std::unordered_set<node_id_t> out{nodes[nid].adj};

  for(auto nid : select_stack)
  {
    out.erase(nid);
  }

  for(auto nid : coalesced_nodes)
  {
    out.erase(nid);
  }
  return out;
}

std::unordered_set<::codegen::assem::Move, RegisterAllocator::MoveHash>
RegisterAllocator::node_moves(node_id_t u)
{
  auto out{nodes[u].moves_list};

  std::erase_if(out, [this](const ::codegen::assem::Move& move) {
    return !active_moves.contains(move) && !worklist_moves.contains(move);
  });

  return out;
}

bool RegisterAllocator::is_move_related(node_id_t n)
{
  return !node_moves(n).empty();
}

void RegisterAllocator::simplify()
{
  // we select a node from the simplify worklist
  assert(simplify_list.size() > 0);
  auto n = simplify_list.front();

  std::cout << std::format("simplifying {}:{}\n", n, helpers::map_temp(nodes[n].t));

  auto s = adjacent(n);
  assert(s.size() == nodes[n].degree || (s.size() == 0 && nodes[n].degree > 1000000));

  simplify_list.pop_front();
  list_push_front(select_stack, n);
  for(auto adj : adjacent(n))
  {
    decrement_degree(adj);
    auto s = adjacent(adj);
    assert(s.size() == nodes[adj].degree || (s.size() == 0 && nodes[adj].degree > 1000000));
  }
}

void RegisterAllocator::decrement_degree(node_id_t nid)
{
  std::cout << std::format("decrementing deg of {}:{}\n", nid, helpers::map_temp(nodes[nid].t));
  auto& data = nodes[nid];
  auto deg = data.degree;
  assert(deg > 0);
  data.degree--;
  if(deg == K)
  {
    // passing from #regs to #regs - 1
    auto adj = adjacent(nid);
    adj.insert(nid);
    enable_moves(adj);
    spill_list.remove(nid);
    if(is_move_related(nid))
    {
      list_push_front(freeze_worklist, nid);
    }
    else
    {
      list_push_front(simplify_list, nid);
    }
  }
}

void RegisterAllocator::enable_moves(const std::unordered_set<node_id_t>& list)
{
  for(auto nid : list)
  {
    for(auto& m : nodes[nid].moves_list)
    {
      active_moves.erase(m);
      worklist_moves.insert(m);
    }
  }
}

void RegisterAllocator::coalesce()
{
  assert(!worklist_moves.empty());
  auto& move = *worklist_moves.begin();
  worklist_moves.erase(move);
  auto x = get_alias(map_tnode[move.src]);
  auto y = get_alias(map_tnode[move.dst]);

  node_id_t u, v;
  // if v is precolored, so is u
  if(is_colored(y))
  {
    u = y;
    v = x;
  }
  else
  {
    u = x;
    v = y;
  }

  std::cout << std::format("maybe coalescing {} <- {}",
                           helpers::map_temp(move.dst),
                           helpers::map_temp(move.src))
            << std::endl;

  auto add_work_list = [this](node_id_t u) {
    if(!is_colored(u) && !is_move_related(u) && nodes[u].degree < K)
    {
      if(auto p = std::find(freeze_worklist.begin(), freeze_worklist.end(), u);
         p != freeze_worklist.end())
      {
        freeze_worklist.erase(p);
        list_push_front(simplify_list, u);
      }
    }
  };

  if(std::find(select_stack.begin(), select_stack.end(), u) != select_stack.end() ||
     std::find(select_stack.begin(), select_stack.end(), v) != select_stack.end()
  )
  {
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO: removing this breaks the algo
    // understand why coalesce is called with u or v on the select stack
    // debug using queens.tig
    return;
  }

  auto george_test = [this](node_id_t t, node_id_t r) {
    // in general nodes a and r can be coalesced if
    // for every neighbor t of a, either t already interferes with r
    // or t is of insignificant degree
    return nodes[t].degree < K || is_colored(t) || edges.contains(edge_t{t, r});
  };

  auto briggs_test = [this](const std::unordered_set<node_id_t>& set) {
    // set will contain the neighbors of the candidate nodes for coalescing
    // here we are testing whether the neighbors having high degree
    // are less than the number of machine registers
    size_t k = 0;
    for(auto nid : set)
    {
      if(nodes[nid].degree >= K)
      {
        k++;
      }
    }
    return k < K;
  };

  if(u == v)
  {
    // already coalesced
    add_work_list(u);
  }
  else if(is_colored(v) || edges.contains(edge_t{u, v}))
  {
    // the move is constrained if both nodes are precolored
    // or there exists an edge between them
    add_work_list(u);
    add_work_list(v);
  }
  else if(is_colored(u))
  {
    // since we don't maintain the adjacency list of a precolored node, we apply george
    auto adj = adjacent(v);
    if(std::all_of(
         adj.begin(), adj.end(), [u, &george_test](node_id_t n) { return george_test(n, u); }))
    {
      // we coalesce v into u, a precolored node
      combine(u, v);
    }
  }
  else
  {
    auto adj_combined = adjacent(u);
    auto adj_v = adjacent(v);
    adj_combined.insert(adj_v.begin(), adj_v.end());

    if(briggs_test(adj_combined))
    {
      combine(u, v);
      add_work_list(u);
    }
    else
    {
      // move is not ready for coalescing
      active_moves.insert(move);
    }
  }
}

RegisterAllocator::node_id_t RegisterAllocator::get_alias(node_id_t n)
{
  if(coalesced_nodes.contains(n))
  {
    assert(nodes[n].alias.has_value());
    return get_alias(nodes[n].alias.value());
  }
  else
  {
    return n;
  }
}

void RegisterAllocator::combine(node_id_t u, node_id_t v)
{
  std::cout << std::format("combining {}:{} <- {}:{}",
                           u,
                           helpers::map_temp(nodes[u].t),
                           v,
                           helpers::map_temp(nodes[v].t))
            << std::endl;

  // remove v from its list
  if(auto p = std::find(freeze_worklist.begin(), freeze_worklist.end(), v);
     p != freeze_worklist.end())
  {
    freeze_worklist.erase(p);
  }
  else
  {
    spill_list.remove(v);
  }

  //////
  auto a = adjacent(u);
  assert(a.size() == nodes[u].degree || (a.size() == 0 && nodes[u].degree > 1000000));
  auto b = adjacent(v);
  assert(b.size() == nodes[v].degree || (b.size() == 0 && nodes[v].degree > 1000000));
  //////////////

  coalesced_nodes.insert(v);
  nodes[v].alias = u;

  // moves of v becomes moves of u
  nodes[u].moves_list.insert(nodes[v].moves_list.begin(), nodes[v].moves_list.end());

  // coalescing v into u could enable moves of v
  enable_moves({v});

  // effectively coalesce
  for(auto t : adjacent(v))
  {
    auto s = adjacent(t);
    assert((s.size() == 0 && nodes[t].degree > 1000000) || (nodes[t].degree == s.size() + 1));

    auto deg_bef = nodes[t].degree;
    static_cast<void>(deg_bef);
    add_edge(t, u);
    auto deg_aft = nodes[t].degree;
    static_cast<void>(deg_aft);
    decrement_degree(t);
    auto deg_aft_aft = nodes[t].degree;
    static_cast<void>(deg_aft_aft);
    s = adjacent(t);
    assert(s.size() == nodes[t].degree || (s.size() == 0 && nodes[t].degree > 1000000));
  }

  // remove u from the freeze worklist if it has high degree
  if(nodes[u].degree >= K)
  {
    auto p = std::find(freeze_worklist.begin(), freeze_worklist.end(), u);
    if(p != freeze_worklist.end())
    {
      freeze_worklist.erase(p);
      list_push_front(spill_list, u);
    }
  }
}

std::vector<std::pair<RegisterAllocator::node_id_t, std::string>>
RegisterAllocator::mapSet(const std::unordered_set<node_id_t>& s)
{
  std::vector<std::pair<RegisterAllocator::node_id_t, std::string>> out;
  for(auto& e : s)
    out.emplace_back(e, helpers::map_temp(nodes[e].t));
  return out;
}

void RegisterAllocator::freeze()
{
  // choose a move-related node with low degree and give up coalescing its moves
  // since it has low degree, it is put in the simplify list
  assert(freeze_worklist.size() > 0);
  auto n = *freeze_worklist.begin();
  freeze_worklist.pop_front();
  list_push_front(simplify_list, n);
  freeze_moves(n);
}

void RegisterAllocator::freeze_moves(node_id_t u)
{
  // the freeze list contains move-related nodes with low degree
  // here u does not belong to the freeze list anymore because we gave up on
  // coalescing its moves
  // hence all the moves related to this node becomes inactive
  // neighbors of u can become non move-related as well

  auto moves = node_moves(u);
  for(auto& move : node_moves(u))
  {
    node_id_t v{};
    if(get_alias(u) == get_alias(map_tnode[move.dst]))
    {
      v = get_alias(map_tnode[move.src]);
    }
    else
    {
      v = get_alias(map_tnode[move.dst]);
    }
    active_moves.erase(move);
    if(!is_move_related(v) && nodes[v].degree < K)
    {
      freeze_worklist.remove(v);
      list_push_front(simplify_list, v);
    }
  }
}

void RegisterAllocator::select_spill()
{
  // TODO: should pick temporaries that are not
  // resulting from the fetches of previously spilled registers
  auto u = *spill_list.begin();
  spill_list.pop_front();
  list_push_front(simplify_list, u);
  freeze_moves(u);
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
      if(is_colored(get_alias(n)))
      {
        ok_colors.erase(nodes[get_alias(n)].color.value());
      }
    }
    if(ok_colors.empty())
    {
      spilled_temps.insert(nodes[top].t);
    }
    else
    {
      nodes[top].color = *ok_colors.begin();
    }
  }

  for(auto nid : coalesced_nodes)
  {
    nodes[nid].color = nodes[get_alias(nid)].color;
  }
}

const std::unordered_set<ir::TempGen::Temp>& RegisterAllocator::perform_allocation()
{
  make_lists();
  do
  {
    if(!simplify_list.empty())
    {
      simplify();
    }
    else if(!worklist_moves.empty())
    {
      coalesce();
    }
    else if(!freeze_worklist.empty())
    {
      freeze();
    }
    else if(!spill_list.empty())
    {
      select_spill();
    }
  } while(!simplify_list.empty() || !worklist_moves.empty() || !freeze_worklist.empty() ||
          !spill_list.empty());

  assign_colors();
  return spilled_temps;
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

  std::unordered_map<node_id_t, Agnode_t*> map;

  for(auto& e : edges)
  {
    auto& n1 = e.first;
    auto& n2 = e.second;
    if(!map.contains(n1))
    {
      std::string descr = helpers::map_temp(nodes[n1].t);
      map[n1] = agnode(graph, descr.data(), true);
    }
    if(!map.contains(n2))
    {
      std::string descr = helpers::map_temp(nodes[n2].t);
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