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
a2 :  BinOpExp(ConstExp(const32), reg, plus)  :: :: 0
a3 :  ConstExp(const32) :: :: 0
a4 :  ConstExp(0) :: :: 0

reg_list : reg | epsilon :: :: 0
const: const32 | const64 | 0

lab : NameExp(label) :: :: 0

reg : ConstExp(const) :: mov new_reg, const :: 0.25
reg : NameExp(label)  :: mov new_reg, label :: 0.25
reg : TempExp(temp) :: :: 0

reg : BinOpExp(reg1, reg2, plus)  :: mov new_reg, reg1; add new_reg, reg2 :: 0.25 + 0.3 = 0.55 
reg : BinOpExp(reg1, reg2, minus) :: mov new_reg, reg1; sub new_reg, reg2 :: 0.25 + 0.3 = 0.55
reg : BinOpExp(reg1, reg2, mul)   :: mov rax, reg1; imul reg2; mov new_reg, rax :: 0.25 + 1 + 0.25 = 1.5 
reg : BinOpExp(reg1, reg2, div)   :: mov rax, reg1; idiv reg2; mov new_reg, rax :: 0.25 + 13 + 0.25 = 13.5 

reg : MemExp(reg1) 	:: mov new_reg, [reg1] :: 0.5
reg : MemExp(a1)   	:: mov new_reg, [a1.reg1 + a1.reg2] :: 0.5
reg : MemExp(a2)  	:: mov new_reg, [a2.reg + a2.const32] :: 0.5
reg : MemExp(a3)   	:: mov new_reg, [a3.const32] :: 0.5

stmt : MoveStmt(reg1, MemExp(reg2)) :: mov reg1, [reg2] :: 0.5
stmt : MoveStmt(reg1, MemExp(a1)) 	:: mov reg1, [a1.reg1 + a1.reg2] :: 0.5
stmt : MoveStmt(reg1, MemExp(a2)) 	:: mov reg1, [a2.reg1 + a2.const32] :: 0.5
stmt : MoveStmt(reg1, MemExp(a3)) 	:: mov reg1, [a3.const32] :: 0.5

stmt : MoveStmt(MemExp(reg1), reg2) :: mov QWORD PTR [reg1], reg2  :: 1
stmt : MoveStmt(MemExp(a1), reg1) 	:: mov QWORD PTR [a1.reg1 + a1.reg2], reg1  :: 1
stmt : MoveStmt(MemExp(a2), reg1) 	:: mov QWORD PTR [a2.reg1 + a2.const32], reg1 :: 1
stmt : MoveStmt(MemExp(a3), reg1) 	:: mov QWORD PTR [a3.const32], reg1 :: 1

stmt : MoveStmt(reg1, reg2)         :: mov reg1, reg2 :: 0.25
stmt : MoveStmt(reg1, a4) 	        :: xor reg1, reg1 :: 0.3
stmt : MoveStmt(reg1, a3) 	        :: mov reg1, a3.const32 :: 0.25

reg : CallExp(reg1, reg_list) 	:: call reg1;             mov new_reg, rax :: 2 + 0.25 = 2.25
reg : CallExp(lab,  reg_list)  	:: call lab.label;  mov new_reg, rax :: 2 + 0.25 = 2.25

stmt : ExpStmt(CallExp(lab, reg_list)) :: call lab.label :: 2
stmt : ExpStmt(CallExp(reg, reg_list)) :: call reg :: 2
stmt : ExpStmt(reg) :: :: 0

stmt : JumpStmt(lab) 	  :: jmp lab.label :: 2
stmt : LabelStmt(label) :: label: :: 0

stmt : CJumpStmt(eq, reg1, reg2, tlab, flab) :: cmp reg1, reg2; je tlab  :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(ne, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jne tlab :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(lt, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jl tlab  :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(gt, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jg tlab  :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(le, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jle tlab :: 0.25 + 0.5 = 0.75
stmt : CJumpStmt(ge, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jge tlab :: 0.25 + 0.5 = 0.75
 
---------------------------------------------------
Instruction selection using maximal munch:

ConstExp(const) -> mov new_reg, const
NameExp(label)  -> mov new_reg, label
TempExp(temp)   -> 

BinOpExp(reg1, reg2, plus)  -> mov new_reg, reg1; add new_reg, reg2
BinOpExp(reg1, reg2, minus) -> mov new_reg, reg1; sub new_reg, reg2
BinOpExp(reg1, reg2, mul)   -> mov rax, reg1; imul reg2; mov new_reg, rax
BinOpExp(reg1, reg2, div)   -> mov rax, reg1; idiv reg2; mov new_reg, rax

MemExp(BinOpExp(ConstExp(const32), reg1, plus))     -> mov new_reg, [reg1 + const32]
MemExp(BinOpExp(reg1, ConstExp(const32), plus))   	-> mov new_reg, [reg1 + const32]
MemExp(BinOpExp(reg1, reg2, plus))   	              -> mov new_reg, [reg1 + reg2]
MemExp(ConstExp(const32))                           -> mov new_reg, [const32]
MemExp(reg1) 	                                      -> mov new_reg, [reg1]

CallExp(NameExp(label),  reg_list)  -> call label;  mov new_reg, rax
CallExp(reg1, reg_list)             -> call reg1; mov new_reg, rax

MoveStmt(MemExp(BinOpExp(ConstExp(const32), reg1, plus)), reg2)     -> mov QWORD PTR [reg1 + const32], reg2
MoveStmt(MemExp(BinOpExp(reg1, ConstExp(const32), plus)), reg2)   	-> mov QWORD PTR [reg1 + const32], reg2
MoveStmt(MemExp(BinOpExp(reg1, reg2, plus)), reg3)   	              -> mov QWORD PTR [reg1 + reg2], reg3
MoveStmt(MemExp(ConstExp(const32)), reg1)   	                      -> mov QWORD PTR [const32], reg1
MoveStmt(MemExp(reg1), reg2)   	                                    -> mov QWORD PTR [reg1], reg2

MoveStmt(reg1, MemExp(BinOpExp(ConstExp(const32), reg2, plus)))     -> mov reg1, [reg2 + const32]
MoveStmt(reg1, MemExp(BinOpExp(reg2, ConstExp(const32), plus)))     -> mov reg1, [reg2 + const32]
MoveStmt(reg1, MemExp(BinOpExp(reg2, reg3, plus)))                  -> mov reg1, [reg2 + reg3]
MoveStmt(reg1, MemExp(ConstExp(const32)))                           -> mov reg1, [const32]
MoveStmt(reg1, MemExp(reg2))                                        -> mov reg1, [reg2]

MoveStmt(reg1, ConstExp(0))   	   -> xor reg1, reg1
MoveStmt(reg1, ConstExp(const32))  -> mov reg1, const32
MoveStmt(reg1, reg2)   	           -> mov reg1, reg2

ExpStmt(CallExp(NameExp(label), reg_list))  -> call label
ExpStmt(CallExp(reg, reg_list))             -> call reg
ExpStmt(reg)                                -> 

JumpStmt(NameExp(label))  -> jmp label
LabelStmt(label) -> label:

CJumpStmt(eq, reg1, reg2, tlab, flab) -> cmp reg1, reg2; je tlab
CJumpStmt(ne, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jne tlab
CJumpStmt(lt, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jl tlab
CJumpStmt(gt, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jg tlab
CJumpStmt(le, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jle tlab
CJumpStmt(ge, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jge tlab

*/

#pragma once
#include <codegen/assem.hpp>
#include <codegen/generator.hpp>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace arch::codegen
{

class MuxMunchGen : public ::codegen::Generator {
  public:
  std::vector<::codegen::assem::Instruction> gen(const ir::tree::Stmt& stmt) override;

  ir::TempGen::Temp operator()(const std::unique_ptr<ir::tree::NameExp>& exp);
  ir::TempGen::Temp operator()(const std::unique_ptr<ir::tree::TempExp>& exp);
  ir::TempGen::Temp operator()(const std::unique_ptr<ir::tree::BinOpExp>& exp);
  ir::TempGen::Temp operator()(const std::unique_ptr<ir::tree::MemExp>& exp);
  ir::TempGen::Temp operator()(const std::unique_ptr<ir::tree::CallExp>& exp);
  ir::TempGen::Temp operator()(const std::unique_ptr<ir::tree::ESeqExp>& exp);
  ir::TempGen::Temp operator()(const std::unique_ptr<ir::tree::ConstExp>& exp);

  void operator()(const std::unique_ptr<ir::tree::MoveStmt>& stmt);
  void operator()(const std::unique_ptr<ir::tree::CJumpStmt>& stmt);
  void operator()(const std::unique_ptr<ir::tree::JumpStmt>& stmt);
  void operator()(const std::unique_ptr<ir::tree::ExpStmt>& stmt);
  void operator()(const std::unique_ptr<ir::tree::SeqStmt>& stmt);
  void operator()(const std::unique_ptr<ir::tree::LabelStmt>& stmt);

  private:
  void munch_store(const ir::tree::MoveStmt& stmt);
  void munch_load(const ir::tree::MoveStmt& stmt);
  void munch_call_exp(const ir::tree::CallExp& exp);
  std::vector<ir::TempGen::Temp> munch_args(const std::vector<ir::tree::Exp>& args);

  bool is_const32(int64_t constant)
  {
    return constant <= std::numeric_limits<int32_t>::max() &&
           constant >= std::numeric_limits<int32_t>::min();
  }

  std::vector<::codegen::assem::Instruction> list;
};

std::string format(std::function<std::string(const ir::TempGen::Temp& t)> mapper,
                   const ::codegen::assem::Instruction& ins);
} // namespace arch::codegen
