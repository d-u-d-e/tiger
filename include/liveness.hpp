#pragma once
#include <flow.hpp>
#include <ir/temp.hpp>

namespace liveness
{
class LivenessAnalyzer {
  public:
  LivenessAnalyzer(flow::FlowGraph& g);
  std::string dump_result();

  private:
  flow::FlowGraph& fg;
};
} // namespace liveness