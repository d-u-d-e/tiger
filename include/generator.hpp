#pragma once

#include "assem.hpp"
#include "ir/tree.hpp"
#include <utility>
#include <vector>

template <typename Generator>
concept IsGenerator = requires(Generator g)
{
  {
    g.gen_impl(std::declval<ir::tree::Stmt>())
    } -> std::same_as<std::vector<assem::Instruction>>;
};

class Generator
{
  public:
  template <IsGenerator Self>
  auto gen(this Self&& self, const ir::tree::Stmt& stmt) -> std::vector<assem::Instruction>
  {
    return std::forward<Self>(self).gen_impl(stmt);
  }
};