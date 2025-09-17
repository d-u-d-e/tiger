#include <liveness.hpp>

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
}

std::string LivenessAnalyzer::dump_result()
{
  std::string out;

  auto map_temp = [](const ir::TempGen::Temp& t) {
    auto mapped = arch::Frame::map_temp(t);
    if(mapped)
    {
      return mapped.value();
    }
    return ir::TempGen::to_string(t);
  };

  auto format_list_of_temps = [&map_temp](const std::list<ir::TempGen::Temp>& a) {
    std::string out("[");
    for(auto i{a.begin()}; i != a.end(); i++)
    {
      out += std::format("{}{}", map_temp(*i), (i != --a.end() ? ", " : ""));
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

} // namespace liveness