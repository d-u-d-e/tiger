#pragma once
#include <concepts>
#include <memory>
#include <vector>

template <typename FrameT>
  requires requires(const FrameT f) {
    typename FrameT::Access;
    { f.formals() } -> std::same_as<std::vector<typename FrameT::Access>>;
  }
struct Level
{
  using Formals = std::vector<typename FrameT::Access>;

  struct Access
  {
    const Level* l{};
    typename FrameT::Access fax;
  };

  Level(const Level* parent, std::unique_ptr<FrameT> f)
    : parent(parent)
    , frame(std::move(f))
  {
    for(Formals const& fs = frame->formals(); auto& formal : fs)
    {
      formals.emplace_back(this, formal);
    }
  }

  std::vector<Access> formals;
  const Level* parent{};
  std::unique_ptr<FrameT> frame;
};