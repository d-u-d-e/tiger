#pragma once                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         terminals: const, label, temp, tlab, flab, plus, minus, mul, div, eq, ne, lt, gt, le, ge

/* 
The following grammar is definitely simple and incomplete for x86-64 ISA,
but a good start to make instruction selection work. For example, arithmetic operations
can access memory, but here we show rules that operate on registers only.
TODO: some temporaries will turn out to be memory locations. They cannot be regarded as registers.
Numbers are appended to each nonterminal on the RHS in order to see how to perform
the code generation. Code to be emitted is shown after the '::' token.
Calls and jumps to labels are always near.

%term const, temp
%term label, tlab, flab
%term plus, minus, mul, div
%term eq, ne, lt, gt, le, ge
%term ConstExp, NameExp, TempExp, BinOpExp, MemExp, CallExp
%term MoveStmt, ExpStmt, JumpStmt, CJumpStmt, LabelStmt
%start goal

%%

goal : goal stmt
goal : stmt

a1 :  BinOpExp(reg1, reg2, plus)
a2 :  BinOpExp(reg, ConstExp(const), plus)
a2 :  BinOpExp(ConstExp(const), reg), plus)
a3 :  ConstExp(const)
a4 :  ConstExp(0)

reg_list : reg | epsilon

lab : NameExp(label)

reg : ConstExp(const) :: move new_reg, const
reg : NameExp(label)  :: move new_reg, label
reg : TempExp(temp)

reg : BinOpExp(reg1, reg2, plus)  :: move new_reg, reg1; add new_reg, reg2	
reg : BinOpExp(reg1, reg2, minus) :: move new_reg, reg1; sub new_reg, reg2
reg : BinOpExp(reg1, reg2, mul)   :: move rax, reg1; imul reg2; move new_reg, rax
reg : BinOpExp(reg1, reg2, div)   :: move rax, reg1; idiv reg2; move new_reg, rax

reg : MemExp(reg1) 	:: move new_reg, [reg1]
reg : MemExp(a1)   	:: move new_reg, [a1.reg1 + a1.reg2]
reg : MemExp(a2)  	:: move new_reg, [a2.reg + const]
reg : MemExp(a3)   	:: move new_reg, [const]

reg : CallExp(reg1, reg_list) 	:: call reg1;             move new_reg, eax
reg : CallExp(lab,  reg_list)  	:: call rel32 lab.label;  move new_reg, eax

stmt : MoveStmt(reg1, reg2) :: move reg1, reg2
stmt : MoveStmt(reg1, a1) 	:: move reg1, [a1.reg1 + a1.reg2]
stmt : MoveStmt(a1, reg2) 	:: move QWORD PTR [a1.reg1 + a1.reg2], reg2
stmt : MoveStmt(reg1, a2) 	:: move reg1, [a2.reg1 + a2.const]
stmt : MoveStmt(a2, reg2) 	:: move QWORD PTR [a2.reg1 + a2.const], reg2
stmt : MoveStmt(reg1, a3) 	:: move reg1, [a3.const]
stmt : MoveStmt(reg1, a4) 	:: xor reg1, reg1

stmt : ExpStmt(CallExp(lab, reg_list)) :: call rel32 lab.label
stmt : ExpStmt(CallExp(reg, reg_list)) :: call reg
stmt : ExpStmt(reg)

stmt : JumpStmt(reg) 		:: jmp reg
stmt : JumpStmt(lab) 	  :: jmp rel32 lab.label

stmt : LabelStmt(label) :: label:

stmt : CJumpStmt(eq, reg1, reg2, tlab, flab) :: cmp reg1, reg2; je tlab
stmt : CJumpStmt(ne, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jne tlab
stmt : CJumpStmt(lt, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jl tlab
stmt : CJumpStmt(gt, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jg tlab
stmt : CJumpStmt(le, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jle tlab
stmt : CJumpStmt(ge, reg1, reg2, tlab, flab) :: cmp reg1, reg2; jge tlab

*/
