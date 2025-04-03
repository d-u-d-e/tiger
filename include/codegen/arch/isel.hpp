/*
--------------------------------------------------- 
Instruction selection using dynamic programming:

The following grammar is definitely coarse for any x86-64 ISA.
It's just a good start to make instruction selection work. For example, arithmetic operations
can access memory, but here we show rules that operate on registers only.
TODO: some temporaries will turn out to be memory locations. They cannot be regarded as registers.
Numbers are appended to each nonterminal on the RHS in order to see how to perform
the code generation. Code to be emitted is shown after the first '::' token.
An instruction "cost" is shown after the second '::' token. This is taken from
https://www.agner.org/optimize/instruction_tables.pdf.
On modern processors, the execution time of a code sequence is not easily predictable from the individual instructions, 
due to interactions between instructions and their shared hardware resources.
The costs used in this algorithm are taken from the 
reciprocal throughput column of the AMD Zen 2 architecture.
Calls and jumps to labels are always near.

%term const32, const64, 0, temp
%term label, tlab, flab
%term plus, minus, mul, div
%term eq, ne, lt, gt, le, ge
%term ConstExp, NameExp, TempExp, BinOpExp, MemExp, CallExp
%term MoveStmt, ExpStmt, JumpStmt, CJumpStmt, LabelStmt
%start goal

%%

goal : goal stmt :: :: 0
goal : stmt :: :: 0

a1 :  BinOpExp(reg1, reg2, plus) :: :: 0
a2 :  BinOpExp(reg, ConstExp(const32), plus)  :: :: 0
a2 :  BinOpExp(ConstExp(const32), reg), plus) :: :: 0
a3 :  ConstExp(const32) :: :: 0
a4 :  ConstExp(0) :: :: 0

reg_list : reg | epsilon :: :: 0
const: const32 | const64 | 0

lab : NameExp(label) :: :: 0

reg : ConstExp(const) :: move new_reg, const :: 0.25
reg : NameExp(label)  :: move new_reg, label :: 0.25
reg : TempExp(temp) :: :: 0

reg : BinOpExp(reg1, reg2, plus)  :: move new_reg, reg1; add new_reg, reg2 :: 0.25 + 0.3 = 0.55 
reg : BinOpExp(reg1, reg2, minus) :: move new_reg, reg1; sub new_reg, reg2 :: 0.25 + 0.3 = 0.55
reg : BinOpExp(reg1, reg2, mul)   :: move rax, reg1; imul reg2; move new_reg, rax :: 0.25 + 1 + 0.25 = 1.5 
reg : BinOpExp(reg1, reg2, div)   :: move rax, reg1; idiv reg2; move new_reg, rax :: 0.25 + 13 + 0.25 = 13.5 

reg : MemExp(reg1) 	:: move new_reg, [reg1] :: 0.5
reg : MemExp(a1)   	:: move new_reg, [a1.reg1 + a1.reg2] :: 0.5
reg : MemExp(a2)  	:: move new_reg, [a2.reg + a2.const32] :: 0.5
reg : MemExp(a3)   	:: move new_reg, [const32] :: 0.5

reg : CallExp(reg1, reg_list) 	:: call reg1;             move new_reg, eax :: 2 + 0.25 = 2.25
reg : CallExp(lab,  reg_list)  	:: call rel32 lab.label;  move new_reg, eax :: 2 + 0.25 = 2.25

stmt : MoveStmt(reg1, reg2) :: move reg1, reg2 :: 0.25
stmt : MoveStmt(reg1, a1) 	:: move reg1, [a1.reg1 + a1.reg2] :: 0.5
stmt : MoveStmt(reg1, a2) 	:: move reg1, [a2.reg1 + a2.const32] :: 0.5
stmt : MoveStmt(reg1, a3) 	:: move reg1, [a3.const32] :: 0.5
stmt : MoveStmt(reg1, a4) 	:: xor reg1, reg1 :: 0.3

stmt : MoveStmt(a1, reg1) 	:: move QWORD PTR [a1.reg1 + a1.reg2], reg1  :: 1
stmt : MoveStmt(a2, reg1) 	:: move QWORD PTR [a2.reg1 + a2.const32], reg1 :: 1
stmt : MoveStmt(a3, reg1) 	:: move QWORD PTR [a3.const32], reg1 :: 1

stmt : ExpStmt(CallExp(lab, reg_list)) :: call rel32 lab.label :: 2
stmt : ExpStmt(CallExp(reg, reg_list)) :: call reg :: 2
stmt : ExpStmt(reg) :: :: 0

stmt : JumpStmt(reg) 		:: jmp reg :: 2
stmt : JumpStmt(lab) 	  :: jmp rel32 lab.label :: 2

stmt : LabelStmt(label) :: label: :: 0

stmt : CJumpStmt(eq, reg1, reg2, tlab, flab) :: cmp reg1, reg2; je tlab  :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(ne, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jne tlab :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(lt, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jl tlab  :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(gt, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jg tlab  :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(le, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jle tlab :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(ge, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jge tlab :: 0.25 + 0.5 = 0.75
 
---------------------------------------------------
Instruction selection using maximal munch:

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

#pragma once
#include <codegen/generator.hpp>
#include <functional>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <optional>

namespace arch::codegen
{

class MuxMunchGen : public ::codegen::Generator {
  public:
  std::vector<::codegen::assem::Instruction>
  gen(const ir::tree::Stmt& stmt) override;

  private:
  ir::TempGen::Temp munch_exp(const ir::tree::Exp& exp);
  void munch_stmt(const ir::tree::Stmt& stmt);

  std::vector<::codegen::assem::Instruction> list;
};

std::string format(
  std::function<std::optional<std::string>(const ir::TempGen::Temp& t)> mapper,
  const ::codegen::assem::Instruction& ins);

} // namespace arch::codegen
