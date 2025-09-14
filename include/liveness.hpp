#pragma once
#include <flow.hpp>

namespace liveness
{
class LivenessAnalyzer {
  public:
  LivenessAnalyzer(const flow::FlowGraph& g)
    : fg(g)
  {
    // TODO
    (void)fg;
  }

  private:
  const flow::FlowGraph& fg;
};
} // namespace liveness