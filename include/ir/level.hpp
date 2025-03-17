#pragma once
#include <arch/frame.hpp>

namespace ir
{

struct Level {
 
  struct Access {
    const Level* l{nullptr};
    arch::Frame::access_t fax;
  };

  Level(const Level* parent, const arch::Frame& f)
    : parent(parent)
    , f(f)
  {
    for(auto& f : f.formals()) {
      formals_.emplace_back(this, f);
    }
  }

  const std::vector<Access>& formals()
  {
    return formals_;
  }

  std::vector<Access> formals_;
  const Level* parent{nullptr};
  arch::Frame f;
};

} // namespace ir