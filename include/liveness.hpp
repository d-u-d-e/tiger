#pragma once
#include "flow.hpp"

namespace liveness
{
class Analyzer
{
  public:
  explicit Analyzer(flow::FlowGraph& g);
  [[nodiscard]] auto dump_result() const -> std::string;
  Analyzer(const Analyzer&) = delete;
  auto operator=(const Analyzer&) -> Analyzer& = delete;
  Analyzer(Analyzer&&) = delete;
  auto operator=(Analyzer&&) -> Analyzer& = delete;
  ~Analyzer() = default;

  private:
  flow::FlowGraph& fg;
};
} // namespace liveness