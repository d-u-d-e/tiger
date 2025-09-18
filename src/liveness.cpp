#include <helpers.hpp>
#include <liveness.hpp>

#if CONFIG_WITH_GRAPHVIZ
#  include <cstdio>
#  include <graphviz/cgraph.h>
#  include <iostream>
#endif

namespace liveness
{

static std::list<ir::TempGen::Temp> union_sorted_lists(const std::list<ir::TempGen::Temp>& a,
                                                       const std::list<ir::TempGen::Temp>& b)
{
  std::list<ir::TempGen::Temp> out;
  auto itera = a.cbegin();
  auto iterb = b.cbegin();
  while(itera != a.end() && iterb != b.end())
  {
    if(*itera == *iterb)
    {
      out.emplace_back(*itera);
      itera++;
      iterb++;
    }
    else if(*itera < *iterb)
    {
      out.emplace_back(*itera);
      itera++;
    }
    else
    {
      out.emplace_back(*iterb);
      iterb++;
    }
  }

  out.insert(out.end(), itera, a.end());
  out.insert(out.end(), iterb, b.end());
  return out;
}

static std::list<ir::TempGen::Temp> diff_sorted_lists(const std::list<ir::TempGen::Temp>& a,
                                                      const std::list<ir::TempGen::Temp>& b)
{
  std::list<ir::TempGen::Temp> out;
  auto itera = a.cbegin();
  auto iterb = b.cbegin();
  while(itera != a.end() && iterb != b.end())
  {
    if(*itera == *iterb)
    {
      itera++;
      iterb++;
    }
    else if(*itera < *iterb)
    {
      out.emplace_back(*itera);
      itera++;
    }
    else
    {
      iterb++;
    }
  }

  out.insert(out.end(), itera, a.end());
  return out;
}

LivenessAnalyzer::LivenessAnalyzer(flow::FlowGraph& g)
  : fg(g)
{
  bool fixed_point{};
  do
  {
    fixed_point = true;
    for(auto& n : fg.get_nodes())
    {
      auto& d = n.data();
      auto live_in_size = d.live_in.size();
      auto live_out_size = d.live_out.size();

      // compute the live in set: use set + (live out set \ def set)
      d.live_in = union_sorted_lists(d.use, diff_sorted_lists(d.live_out, d.def));

      // compute the live out set: (for all successors: + live_in)
      std::list<ir::TempGen::Temp> live_out;
      for(auto succ_id : fg.succ(n.id()))
      {
        live_out = union_sorted_lists(live_out, fg.get_node(succ_id).data().live_in);
      };
      d.live_out = std::move(live_out);

      fixed_point =
        fixed_point && (live_in_size == d.live_in.size() && live_out_size == d.live_out.size());
    }
    //std::cout << dump_result() << std::endl;
  } while(!fixed_point);

  // next step: build the interference graph
  make_interference_graph();
}

std::string LivenessAnalyzer::dump_result()
{
  std::string out;
  auto format_list_of_temps = [](const std::list<ir::TempGen::Temp>& a) {
    std::string out("[");
    for(auto i{a.begin()}; i != a.end(); i++)
    {
      out += std::format("{}{}", helpers::map_temp(*i), (i != --a.end() ? ", " : ""));
    }
    out += "]\n";
    return out;
  };

  for(auto& n : fg.get_nodes())
  {
    auto& d = n.data();
    out += std::format("live IN temporaries at node {}: ", n.id());
    out += format_list_of_temps(d.live_in);
    out += std::format("live OUT temporaries at node {}: ", n.id());
    out += format_list_of_temps(d.live_out);
  }
  return out;
}

void LivenessAnalyzer::make_interference_graph()
{
  for(auto& n : fg.get_nodes())
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
void LivenessAnalyzer::render_igraph_dot(const std::string& name, const std::string& filename)
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

} // namespace liveness