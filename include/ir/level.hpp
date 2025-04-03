#pragma once
#include <codegen/arch/frame.hpp>
#include <vector>

namespace ir
{

struct Level {

  public:
  struct Access {
    const Level* l{nullptr};
    arch::Frame::Access fax;
  };

  Level(const Level* parent, const arch::Frame& f)
    : parent(parent)
    , f(f)
  {
    for(auto& f : f.formals()) {
      formals.emplace_back(this, f);
    }
  }

  std::vector<Access> formals;
  const Level* parent{nullptr};
  arch::Frame f;
};

} // namespace ir