#pragma once
#include <codegen/arch.hpp>
#include <memory>
#include <utility>
#include <vector>

namespace ir
{

struct Level {
  struct Access {
    const Level* l{};
    arch::Frame::Access fax;
  };

  Level(const Level* parent, std::unique_ptr<arch::Frame> f)
    : parent(parent)
    , frame(std::move(f))
  {
    for(auto& formal : frame->formals())
    {
      formals.emplace_back(this, formal);
    }
  }

  std::vector<Access> formals;
  const Level* parent{};
  std::unique_ptr<arch::Frame> frame;
};

} // namespace ir