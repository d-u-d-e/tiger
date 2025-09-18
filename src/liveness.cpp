#include <helpers.hpp>
#include <liveness.hpp>

namespace liveness
{

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
      d.live_in = helpers::union_sorted_lists(d.use, helpers::diff_sorted_lists(d.live_out, d.def));

      // compute the live out set: (for all successors: + live_in)
      std::list<ir::TempGen::Temp> live_out;
      for(auto succ_id : fg.succ(n.id()))
      {
        live_out = helpers::union_sorted_lists(live_out, fg.get_node(succ_id).data().live_in);
      };
      d.live_out = std::move(live_out);

      fixed_point =
        fixed_point && (live_in_size == d.live_in.size() && live_out_size == d.live_out.size());
    }
  } while(!fixed_point);
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

} // namespace liveness