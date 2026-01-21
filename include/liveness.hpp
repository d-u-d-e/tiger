#pragma once
#include "flow.hpp"

namespace liveness
{
class Analyzer
{
  public:
  Analyzer(flow::FlowGraph& g);
  std::string dump_result();

  private:
  flow::FlowGraph& fg;
};
} // namespace liveness