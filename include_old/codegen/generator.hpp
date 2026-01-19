#pragma once
#include <codegen/assem.hpp>
#include <ir/tree.hpp>
#include <vector>

namespace codegen
{

class Generator {
  public:
  virtual std::vector<assem::Instruction> gen(const ir::tree::Stmt& stmt) = 0;
};

} // namespace codegen