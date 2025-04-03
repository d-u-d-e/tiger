#include <codegen/arch/isel.hpp>

namespace arch::codegen
{

std::vector<::codegen::assem::Instruction>
MuxMunchGen::gen(const ir::tree::Stmt& stmt)
{
  // TODO
  (void)stmt;
  list.clear();
  return list;
}

ir::TempGen::Temp MuxMunchGen::munch_exp(const ir::tree::Exp& exp)
{
  // TODO
  (void)exp;
  return ir::TempGen::new_temp();
}

void MuxMunchGen::munch_stmt(const ir::tree::Stmt& stmt)
{
  // TODO
  (void)stmt;
}

} // namespace arch::codegen