#include "arch/x86_64/generator.hpp"
#include "arch/x86_64/frame.hpp"
#include <cassert>
#include <cstddef>
#include <format>
#include <utility>
#include <variant>
#include <vector>

namespace arch
{

std::vector<assem::Instruction> X86Generator::gen(const ir::tree::Stmt& stmt)
{
  list.clear();
  std::visit(*this, stmt);
  return list;
}

TempGen::Temp X86Generator::operator()(const std::unique_ptr<ir::tree::NameExp>& exp)
{
  // NameExp(label) -> lea new_reg, label
  auto result = TempGen::new_temp();
  list.emplace_back(assem::Oper{
    .assem = std::format("lea  `d0, [rip+{}]\n", exp->label.str()), .dst{result}, .src{}, .jmp{}});
  return result;
}

TempGen::Temp X86Generator::operator()(const std::unique_ptr<ir::tree::TempExp>& exp)
{
  // TempExp(temp) ->
  return exp->temp;
}

TempGen::Temp X86Generator::operator()(const std::unique_ptr<ir::tree::BinOpExp>& exp)
{
  auto left = std::visit(*this, exp->left);
  auto right = std::visit(*this, exp->right);
  auto result = TempGen::new_temp();
  if(exp->op == ir::tree::BinaryOp::plus)
  {
    // BinOpExp(reg1, reg2, plus) -> mov new_reg, reg1; add new_reg, reg2
    list.emplace_back(assem::Move{
      .assem = "mov  `d0, `s0\n",
      .dst = result,
      .src = left,
    });
    list.emplace_back(assem::Oper{
      .assem{"add  `d0, `s1\n"},
      .dst{result},
      .src{result, right},
      .jmp{},
    });
  }
  else if(exp->op == ir::tree::BinaryOp::minus)
  {
    // BinOpExp(reg1, reg2, minus)  -> mov new_reg, reg1; sub new_reg, reg2
    list.emplace_back(assem::Move{
      .assem = "mov  `d0, `s0\n",
      .dst = result,
      .src = left,
    });
    list.emplace_back(assem::Oper{
      .assem{"sub  `d0, `s1\n"},
      .dst{result},
      .src{result, right},
      .jmp{},
    });
  }
  else if(exp->op == ir::tree::BinaryOp::mul)
  {
    // BinOpExp(reg1, reg2, mul) -> mov rax, reg1; imul reg2; mov new_reg, rax
    list.emplace_back(assem::Move{
      .assem = "mov  `d0, `s0\n",
      .dst = X86Frame::RAX,
      .src = left,
    });
    list.emplace_back(assem::Oper{
      .assem{"imul `s0\n"},
      .dst{X86Frame::RAX, X86Frame::RDX},
      .src{right, X86Frame::RAX},
      .jmp{},
    });
    list.emplace_back(assem::Move{.assem = "mov  `d0, `s0\n", .dst = result, .src = X86Frame::RAX});
  }
  else if(exp->op == ir::tree::BinaryOp::div)
  {
    // BinOpExp(reg1, reg2, div) -> mov rax, reg1; cqo; idiv reg2; mov new_reg, rax
    list.emplace_back(assem::Move{.assem = "mov  `d0, `s0\n", .dst = X86Frame::RAX, .src = left});
    list.emplace_back(assem::Oper{
      .assem{"cqo\n"},
      .dst{X86Frame::RDX},
      .src{X86Frame::RAX},
      .jmp{},
    });
    list.emplace_back(assem::Oper{
      .assem{"idiv `s0\n"},
      .dst{X86Frame::RAX, X86Frame::RDX},
      .src{right, X86Frame::RAX},
      .jmp{},
    });
    list.emplace_back(assem::Move{.assem = "mov  `d0, `s0\n", .dst = result, .src = X86Frame::RAX});
  }
  else
  {
    // there are other binops that should be handled, but our frontend will not generate them
    assert(false);
  }
  return result;
}

TempGen::Temp X86Generator::operator()(const std::unique_ptr<ir::tree::MemExp>& exp)
{
  if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(exp->a))
  {
    // MemExp(ConstExp(const32)) -> mov new_reg, [const32]
    auto& constexp = std::get<std::unique_ptr<ir::tree::ConstExp>>(exp->a);
    if(is_const32(constexp->v))
    {
      auto result = TempGen::new_temp();
      auto constant = constexp->v;
      list.emplace_back(assem::Oper{
        .assem{std::format("mov  `d0, [{}]\n", constant)},
        .dst{result},
        .src{},
        .jmp{},
      });
      return result;
    }
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::BinOpExp>>(exp->a))
  {
    // MemExp(BinOpExp(ConstExp(const32), reg1, plus)) -> mov new_reg, [reg1+const32]
    // MemExp(BinOpExp(reg1, ConstExp(const32), plus)) -> mov new_reg, [reg1+const32]
    // MemExp(BinOpExp(reg1, reg2, plus)) -> mov new_reg, [reg1+reg2]

    auto& binexp = std::get<std::unique_ptr<ir::tree::BinOpExp>>(exp->a);
    if(binexp->op == ir::tree::BinaryOp::plus)
    {
      if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(binexp->left))
      {
        auto constant = std::get<std::unique_ptr<ir::tree::ConstExp>>(binexp->left)->v;
        if(is_const32(constant))
        {
          auto reg1 = std::visit(*this, binexp->right);
          auto result = TempGen::new_temp();
          list.emplace_back(assem::Oper{
            .assem{std::format("mov  `d0, [`s0{:+}]\n", constant)},
            .dst{result},
            .src{reg1},
            .jmp{},
          });
          return result;
        }
      }
      else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(binexp->right))
      {
        auto constant = std::get<std::unique_ptr<ir::tree::ConstExp>>(binexp->right)->v;
        if(is_const32(constant))
        {
          auto reg1 = std::visit(*this, binexp->left);
          auto result = TempGen::new_temp();
          list.emplace_back(assem::Oper{
            .assem{std::format("mov  `d0, [`s0{:+}]\n", constant)},
            .dst{result},
            .src{reg1},
            .jmp{},
          });
          return result;
        }
      }
      auto reg1 = std::visit(*this, binexp->left);
      auto reg2 = std::visit(*this, binexp->right);
      auto result = TempGen::new_temp();
      list.emplace_back(assem::Oper{
        .assem{"mov  `d0, [`s0+`s1]\n"},
        .dst{result},
        .src{reg1, reg2},
        .jmp{},
      });
      return result;
    }
  }

  // MemExp(reg1) -> mov new_reg, [reg1]
  auto reg1 = std::visit(*this, exp->a);
  auto result = TempGen::new_temp();
  list.emplace_back(assem::Oper{
    .assem{"mov  `d0, [`s0]\n"},
    .dst{result},
    .src{reg1},
    .jmp{},
  });
  return result;
}

TempGen::Temp X86Generator::operator()(const std::unique_ptr<ir::tree::CallExp>&)
{
  // this is handled while munching expression statements or mov statements
  assert(false);
  std::unreachable();
}

TempGen::Temp X86Generator::operator()(const std::unique_ptr<ir::tree::ESeqExp>&)
{
  // the tree has been canonicalized
  assert(false);
  std::unreachable();
}

TempGen::Temp X86Generator::operator()(const std::unique_ptr<ir::tree::ConstExp>& exp)
{
  // ConstExp(const) -> mov new_reg, const
  auto result = TempGen::new_temp();
  list.emplace_back(
    assem::Oper{.assem = std::format("mov  `d0, {}\n", exp->v), .dst{result}, .src{}, .jmp{}});
  return result;
}

void X86Generator::operator()(const std::unique_ptr<ir::tree::MoveStmt>& stmt)
{
  if(std::holds_alternative<std::unique_ptr<ir::tree::MemExp>>(stmt->left))
  {
    return munch_store(*stmt);
  }

  if(std::holds_alternative<std::unique_ptr<ir::tree::MemExp>>(stmt->right))
  {
    return munch_load(*stmt);
  }

  // must be moving to a register then
  assert(std::holds_alternative<std::unique_ptr<ir::tree::TempExp>>(stmt->left));
  auto temp = std::get<std::unique_ptr<ir::tree::TempExp>>(stmt->left)->temp;

  if(std::holds_alternative<std::unique_ptr<ir::tree::CallExp>>(stmt->right))
  {
    // MoveStmt(TempExp(temp), CallExp(NameExp(label), reg_list)) -> call label; mov temp, rax
    // MoveStmt(TempExp(temp), CallExp(reg, reg_list)) -> call reg; mov temp, rax
    munch_call_exp(*std::get<std::unique_ptr<ir::tree::CallExp>>(stmt->right));
    list.emplace_back(assem::Move{.assem = "mov  `d0, `s0\n", .dst = temp, .src = X86Frame::RAX});
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(stmt->right))
  {
    auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(stmt->right)->v;
    if(c == 0)
    {
      // MoveStmt(reg1, ConstExp(0)) -> xor reg1, reg1
      list.emplace_back(assem::Oper{
        .assem{"xor  `s0, `s0\n"},
        .dst{temp},
        .src{temp},
        .jmp{},
      });
    }
    else
    {
      // MoveStmt(reg1, ConstExp(const32)) -> mov reg1, const32
      list.emplace_back(assem::Oper{
        .assem{std::format("mov  `d0, {}\n", c)},
        .dst{temp},
        .src{},
        .jmp{},
      });
    }
  }
  else
  {
    // MoveStmt(reg1, reg2) -> mov reg1, reg2
    auto reg2 = std::visit(*this, stmt->right);
    list.emplace_back(assem::Move{.assem = "mov  `d0, `s0\n", .dst = temp, .src = reg2});
  }
}

void X86Generator::operator()(const std::unique_ptr<ir::tree::CJumpStmt>& stmt)
{
  /*
  CJumpStmt(eq, reg1, reg2, tlab, flab) -> cmp reg1, reg2; je tlab
  CJumpStmt(ne, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jne tlab
  CJumpStmt(lt, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jl tlab
  CJumpStmt(gt, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jg tlab
  CJumpStmt(le, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jle tlab
  CJumpStmt(ge, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jge tlab
  */

  auto reg1 = std::visit(*this, stmt->lexp);
  auto reg2 = std::visit(*this, stmt->rexp);

  list.emplace_back(assem::Oper{
    .assem{"cmp  `s0, `s1\n"},
    .dst{},
    .src{reg1, reg2},
    .jmp{},
  });

  std::string assem{};
  auto tlabel = stmt->tlabel.str();
  switch(stmt->op)
  {
  case ir::tree::RelOp::eq: {
    assem = std::format("je   {}\n", tlabel);
    break;
  }
  case ir::tree::RelOp::ne: {
    assem = std::format("jne  {}\n", tlabel);
    break;
  }
  case ir::tree::RelOp::lt: {
    assem = std::format("jl   {}\n", tlabel);
    break;
  }
  case ir::tree::RelOp::gt: {
    assem = std::format("jg   {}\n", tlabel);
    break;
  }
  case ir::tree::RelOp::le: {
    assem = std::format("jle  {}\n", tlabel);
    break;
  }
  case ir::tree::RelOp::ge: {
    assem = std::format("jge  {}\n", tlabel);
    break;
  }
  default:
    assert(false);
  }

  list.emplace_back(assem::Oper{
    .assem{assem},
    .dst{},
    .src{},
    .jmp{std::vector{stmt->tlabel, stmt->flabel}},
  });
}

void X86Generator::operator()(const std::unique_ptr<ir::tree::JumpStmt>& stmt)
{
  // JumpStmt(NameExp(label))  -> jmp label
  auto& jmp = stmt->a;
  assert(std::holds_alternative<std::unique_ptr<ir::tree::NameExp>>(jmp));
  auto ljmp = std::get<std::unique_ptr<ir::tree::NameExp>>(jmp)->label;
  list.emplace_back(assem::Oper{
    .assem = std::format("jmp  {}\n", ljmp.str()),
    .dst{},
    .src{},
    .jmp{stmt->labels},
  });
}

void X86Generator::operator()(const std::unique_ptr<ir::tree::ExpStmt>& stmt)
{
  if(std::holds_alternative<std::unique_ptr<ir::tree::CallExp>>(stmt->exp))
  {
    // ExpStmt(CallExp(NameExp(label), reg_list))
    munch_call_exp(*std::get<std::unique_ptr<ir::tree::CallExp>>(stmt->exp));
  }
  else
  {
    // ExpStmt(reg) ->
    // discard the result
    (void)std::visit(*this, stmt->exp);
  }
}

void X86Generator::operator()(const std::unique_ptr<ir::tree::SeqStmt>&)
{
  // the tree has been canonicalized
  assert(false);
  std::unreachable();
}

void X86Generator::operator()(const std::unique_ptr<ir::tree::LabelStmt>& stmt)
{
  // LabelStmt(label) -> label:
  list.emplace_back(
    assem::Label{.assem{std::format("{}:\n", stmt->label.str())}, .label{stmt->label}});
}

void X86Generator::munch_store(const ir::tree::MoveStmt& stmt)
{
  assert(std::holds_alternative<std::unique_ptr<ir::tree::MemExp>>(stmt.left));
  auto& memexp = std::get<std::unique_ptr<ir::tree::MemExp>>(stmt.left);

  if(std::holds_alternative<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a))
  {
    // MoveStmt(MemExp(BinOpExp(ConstExp(const32), reg1, plus)), reg2) -> mov QWORD PTR [reg1+const32], reg2
    // MoveStmt(MemExp(BinOpExp(reg1, ConstExp(const32), plus)), reg2) -> mov QWORD PTR [reg1+const32], reg2
    // MoveStmt(MemExp(BinOpExp(reg1, reg2, plus)), reg3) -> mov QWORD PTR [reg1+reg2], reg3
    auto& binopexp = std::get<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a);
    if(binopexp->op == ir::tree::BinaryOp::plus)
    {
      if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left))
      {
        auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left)->v;
        if(is_const32(c))
        {
          auto reg1 = std::visit(*this, binopexp->right);
          auto reg2 = std::visit(*this, stmt.right);
          list.emplace_back(assem::Oper{
            .assem{std::format("mov  QWORD PTR [`s0{:+}], `s1\n", c)},
            .dst{},
            .src{reg1, reg2},
            .jmp{},
          });
          return;
        }
      }
      else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(binopexp->right))
      {
        auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->right)->v;
        if(is_const32(c))
        {
          auto reg1 = std::visit(*this, binopexp->left);
          auto reg2 = std::visit(*this, stmt.right);
          list.emplace_back(assem::Oper{
            .assem{std::format("mov  QWORD PTR [`s0{:+}], `s1\n", c)},
            .dst{},
            .src{reg1, reg2},
            .jmp{},
          });
          return;
        }
      }

      auto reg1 = std::visit(*this, binopexp->left);
      auto reg2 = std::visit(*this, binopexp->right);
      auto reg3 = std::visit(*this, stmt.right);
      list.emplace_back(assem::Oper{
        .assem{"mov  QWORD PTR [`s0+`s1], `s2\n"},
        .dst{},
        .src{reg1, reg2, reg3},
        .jmp{},
      });
      return;
    }
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(memexp->a) &&
          is_const32(std::get<std::unique_ptr<ir::tree::ConstExp>>(memexp->a)->v))
  {
    // MoveStmt(MemExp(ConstExp(const32)), reg1) -> mov QWORD PTR [const32], reg1
    auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(memexp->a)->v;
    auto reg1 = std::visit(*this, stmt.right);
    list.emplace_back(assem::Oper{
      .assem{std::format("mov  QWORD PTR [{}], `s0\n", c)},
      .dst{},
      .src{reg1},
      .jmp{},
    });
    return;
  }

  // MoveStmt(MemExp(reg1), reg2) -> mov QWORD PTR [reg1], reg2
  auto reg1 = std::visit(*this, memexp->a);
  auto reg2 = std::visit(*this, stmt.right);
  list.emplace_back(assem::Oper{
    .assem{"mov  QWORD PTR [`s0], `s1\n"},
    .dst{},
    .src{reg1, reg2},
    .jmp{},
  });
}

void X86Generator::munch_load(const ir::tree::MoveStmt& stmt)
{
  assert(std::holds_alternative<std::unique_ptr<ir::tree::MemExp>>(stmt.right));
  auto& memexp = std::get<std::unique_ptr<ir::tree::MemExp>>(stmt.right);

  if(std::holds_alternative<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a))
  {
    // MoveStmt(reg1, MemExp(BinOpExp(ConstExp(const32), reg2, plus))) -> mov reg1, [reg2+const32]
    // MoveStmt(reg1, MemExp(BinOpExp(reg2, ConstExp(const32), plus))) -> mov reg1, [reg2+const32]
    // MoveStmt(reg1, MemExp(BinOpExp(reg2, reg3, plus))) -> mov reg1, [reg2+reg3]
    auto& binopexp = std::get<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a);
    if(binopexp->op == ir::tree::BinaryOp::plus)
    {
      if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left))
      {
        auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left)->v;
        if(is_const32(c))
        {
          auto reg1 = std::visit(*this, stmt.left);
          auto reg2 = std::visit(*this, binopexp->right);
          list.emplace_back(assem::Oper{
            .assem{std::format("mov  `d0, [`s0{:+}]\n", c)},
            .dst{reg1},
            .src{reg2},
            .jmp{},
          });
          return;
        }
      }
      else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(binopexp->right))
      {
        auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->right)->v;
        if(is_const32(c))
        {
          auto reg1 = std::visit(*this, stmt.left);
          auto reg2 = std::visit(*this, binopexp->left);
          list.emplace_back(assem::Oper{
            .assem{std::format("mov  `d0, [`s0{:+}]\n", c)},
            .dst{reg1},
            .src{reg2},
            .jmp{},
          });
          return;
        }
      }
      auto reg1 = std::visit(*this, stmt.left);
      auto reg2 = std::visit(*this, binopexp->left);
      auto reg3 = std::visit(*this, binopexp->right);
      list.emplace_back(assem::Oper{
        .assem{"mov  `d0, [`s0+`s1]\n"},
        .dst{reg1},
        .src{reg2, reg3},
        .jmp{},
      });
      return;
    }
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(memexp->a) &&
          is_const32(std::get<std::unique_ptr<ir::tree::ConstExp>>(memexp->a)->v))
  {
    // MoveStmt(reg1, MemExp(ConstExp(const32))) -> mov reg1, [const32]
    auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(memexp->a)->v;
    auto reg1 = std::visit(*this, stmt.left);
    list.emplace_back(assem::Oper{
      .assem{std::format("mov  `d0, [{}]\n", c)},
      .dst{reg1},
      .src{},
      .jmp{},
    });
    return;
  }

  // MoveStmt(reg1, MemExp(reg2)) -> mov reg1, [reg2]
  auto reg1 = std::visit(*this, stmt.left);
  auto reg2 = std::visit(*this, memexp->a);
  list.emplace_back(assem::Oper{
    .assem{"mov  `d0, [`s0]\n"},
    .dst{reg1},
    .src{reg2},
    .jmp{},
  });
}

void X86Generator::munch_call_exp(const ir::tree::CallExp& exp)
{
  // CallExp(NameExp(label),  reg_list) -> call label
  // A call instruction can trash the caller saved registers, so any live value should
  // not be contained in those registers. RAX is also trashed. FP and SP are saved by the prologue
  // and restored by the epilogue.

  auto trashed = std::vector({X86Frame::RAX});
  std::copy(
    X86Frame::caller_saved.begin(), X86Frame::caller_saved.end(), std::back_inserter(trashed));

  assert(std::holds_alternative<std::unique_ptr<ir::tree::NameExp>>(exp.fun));
  auto ljmp = std::get<std::unique_ptr<ir::tree::NameExp>>(exp.fun)->label;
  list.emplace_back(assem::Oper{
    .assem = std::format("call {}\n", ljmp.str()),
    .dst{std::move(trashed)},
    .src{munch_args(exp.args)},
    .jmp{},
  });
}

std::vector<TempGen::Temp> X86Generator::munch_args(const std::vector<ir::tree::Exp>& args)
{
  std::vector<TempGen::Temp> srcs_call;
  auto k = X86Frame::params_on_regs.size();

  for(size_t i = 0; i < std::min(k, args.size()); i++)
  {
    auto t = std::visit(*this, args[i]);
    srcs_call.push_back(X86Frame::params_on_regs[i]);
    list.emplace_back(
      assem::Move{.assem{"mov  `d0, `s0\n"}, .dst = X86Frame::params_on_regs[i], .src = t});
  }

  for(size_t i = k; i < args.size(); i++)
  {
    auto t = std::visit(*this, args[i]);
    // the instruction will be patched later by proc_entry_exit2, since we need to alloc space on the current
    // stack frame for outgoing parameters, but this space should be calculated based on all calls
    list.emplace_back(assem::Oper{.assem{"*\n"}, .dst{}, .src{X86Frame::FP, t}, .jmp{}});
  }
  return srcs_call;
}
} // namespace arch