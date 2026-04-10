#include "liveness.hpp"
#include "utils/list.hpp"

namespace liveness
{

Analyzer::Analyzer(flow::FlowGraph& g)
  : fg(g)
{
  bool fixed_point{false};
  while(!fixed_point)
  {
    fixed_point = true;
    for(auto& n : fg.get_nodes())
    {
      auto& d = fg[n].data();
      auto live_in_size = d.live_in.size();
      auto live_out_size = d.live_out.size();

      // compute the live in set: use set + (live out set \ def set)
      d.live_in = utils::union_sorted_lists(d.use, utils::diff_sorted_lists(d.live_out, d.def));

      // compute the live out set: (for all successors: + live_in)
      std::list<TempGen::Temp> live_out;
      for(auto succ_id : fg.succ(n))
      {
        live_out = utils::union_sorted_lists(live_out, fg[succ_id].data().live_in);
      };
      d.live_out = std::move(live_out);

      fixed_point =
        fixed_point && (live_in_size == d.live_in.size() && live_out_size == d.live_out.size());
    }
  }
}

auto Analyzer::dump_result() const -> std::string
{
  std::string out;
  const auto& temp_mapper = fg.get_temporary_mapper();

  auto format_list_of_temps = [&temp_mapper](const std::list<TempGen::Temp>& a) {
    std::string out("[");
    for(auto i{a.begin()}; i != a.end(); i++)
    {
      out += std::format("{}{}", temp_mapper(*i), (i != --a.end() ? ", " : ""));
    }
    out += "]\n";
    return out;
  };

  for(auto& n : fg.get_nodes())
  {
    auto& d = fg[n].data();
    out += std::format("live IN temporaries at node {}: ", n);
    out += format_list_of_temps(d.live_in);
    out += std::format("live OUT temporaries at node {}: ", n);
    out += format_list_of_temps(d.live_out);
  }
  return out;
}

} // namespace liveness