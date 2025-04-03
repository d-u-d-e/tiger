#include <codegen/arch/isel.hpp>
#include <functional>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <optional>

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
  /*
  ConstExp(const) -> move new_reg, const
  NameExp(label)  -> move new_reg, label
  TempExp(temp)   -> 

  BinOpExp(reg1, reg2, plus)  -> move new_reg, reg1; add new_reg, reg2
  BinOpExp(reg1, reg2, minus) -> move new_reg, reg1; sub new_reg, reg2
  BinOpExp(reg1, reg2, mul)   -> move rax, reg1; imul reg2; move new_reg, rax
  BinOpExp(reg1, reg2, div)   -> move rax, reg1; idiv reg2; move new_reg, rax

  MemExp(BinOpExp(ConstExp(const32), reg1), plus))    -> move new_reg, [reg1 + const32]
  MemExp(BinOpExp(reg1, ConstExp(const32), plus))   	-> move new_reg, [reg1 + const32]
  MemExp(BinOpExp(reg1, reg2, plus))   	              -> move new_reg, [reg1 + reg2]
  MemExp(ConstExp(const32))                           -> move new_reg, [const32]
  MemExp(reg1) 	                                      -> move new_reg, [reg1]

  CallExp(NameExp(label),  reg_list)  -> call label;  move new_reg, eax
  CallExp(reg1, reg_list)             -> call reg1; move new_reg, eax
  */

  // instr templates:
  // move {}, {}
  // call {}
  // add {}, {}
  // sub {}, {}
  // idiv {}
  // imul {}

  // TODO
  (void)exp;
  return ir::TempGen::new_temp();
}

void MuxMunchGen::munch_stmt(const ir::tree::Stmt& stmt)
{
  /*
  MoveStmt(MemExp(BinOpExp(ConstExp(const32), reg1), plus)), reg2)    -> move QWORD PTR [reg1 + const32], reg2
  MoveStmt(MemExp(BinOpExp(reg1, ConstExp(const32), plus)), reg2)   	-> move QWORD PTR [reg1 + const32], reg2
  MoveStmt(MemExp(ConstExp(const32)), reg1)   	                      -> move QWORD PTR [const32], reg1
  MoveStmt(reg1, MemExp(BinOpExp(ConstExp(const32), reg2), plus)))    -> move reg1, [reg2 + const32]
  MoveStmt(reg1, MemExp(BinOpExp(reg2, ConstExp(const32)), plus)))    -> move reg1, [reg2 + const32]
  MoveStmt(reg1, MemExp(ConstExp(0)))   	                            -> xor reg1, reg1
  MoveStmt(reg1, MemExp(ConstExp(const32)))                           -> move reg1, [const32]
  MoveStmt(reg1, reg2)   	                                            -> move reg1, reg2

  ExpStmt(CallExp(NameExp(label), reg_list))  -> call rel32 label
  ExpStmt(CallExp(reg, reg_list))             -> call reg
  ExpStmt(reg)                                -> 

  JumpStmt(NameExp(label))  -> jmp rel32 label
  JumpStmt(reg)             -> jmp reg

  LabelStmt(label) -> label:

  CJumpStmt(eq, reg1, reg2, tlab, flab) -> cmp reg1, reg2; je tlab
  CJumpStmt(ne, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jne tlab
  CJumpStmt(lt, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jl tlab
  CJumpStmt(gt, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jg tlab
  CJumpStmt(le, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jle tlab
  CJumpStmt(ge, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jge tlab
  */

  // TODO
  (void)stmt;
}

std::string format(
  std::function<std::optional<std::string>(const ir::TempGen::Temp& t)> mapper,
  const ::codegen::assem::Instruction& ins)
{
  using namespace ::codegen::assem;

  (void)mapper; // TODO

  if(std::holds_alternative<Oper>(ins)) {
    auto& cins = std::get<Oper>(ins);
    // TODO
    (void)cins;
  }
  if(std::holds_alternative<Move>(ins)) {
    auto& cins = std::get<Move>(ins);
    // TODO
    (void)cins;
  }
  else if(std::holds_alternative<Label>(ins)) {
    auto& cins = std::get<Label>(ins);
    // TODO
    (void)cins;
  }
  return "?";
}

} // namespace arch::codegen