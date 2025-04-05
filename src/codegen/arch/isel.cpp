#include <cassert>
#include <codegen/arch/frame.hpp>
#include <codegen/arch/isel.hpp>
#include <codegen/assem.hpp>
#include <cstdint>
#include <format>
#include <functional>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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
  if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(exp)) {
    // ConstExp(const) -> move new_reg, const
    auto result = ir::TempGen::new_temp();
    list.emplace_back(::codegen::assem::Oper{
      .assem = std::format(
        "move `d0, {}", std::get<std::unique_ptr<ir::tree::ConstExp>>(exp)->v),
      .dst{result},
      .src{},
      .jmp{}});
    return result;
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::NameExp>>(exp)) {
    // NameExp(label) -> move new_reg, label
    auto result = ir::TempGen::new_temp();
    list.emplace_back(::codegen::assem::Oper{
      .assem = std::format(
        "move `d0, {}",
        std::get<std::unique_ptr<ir::tree::NameExp>>(exp)->label.str()),
      .dst{result},
      .src{},
      .jmp{}});
    return result;
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::TempExp>>(exp)) {
    // TempExp(temp) ->
    return std::get<std::unique_ptr<ir::tree::TempExp>>(exp)->temp;
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::BinOpExp>>(exp)) {
    return munch_binop_exp(*std::get<std::unique_ptr<ir::tree::BinOpExp>>(exp));
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::MemExp>>(exp)) {
    return munch_mem_exp(*std::get<std::unique_ptr<ir::tree::MemExp>>(exp));
  }
  assert(false);
  std::unreachable();
}

ir::TempGen::Temp MuxMunchGen::munch_mem_exp(const ir::tree::MemExp& exp)
{
  if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(exp.a)) {
    // MemExp(ConstExp(const32)) -> move new_reg, [const32]
    auto const32 = std::get<std::unique_ptr<ir::tree::ConstExp>>(exp.a)->v;
    if(const32 <= std::numeric_limits<uint32_t>::max()) {
      auto result = ir::TempGen::new_temp();
      list.emplace_back(::codegen::assem::Oper{
        .assem{std::format("move `d0, [{}]", const32)},
        .dst{result},
        .src{},
        .jmp{},
      });
      return result;
    }
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::BinOpExp>>(exp.a)) {
    auto& binexp = std::get<std::unique_ptr<ir::tree::BinOpExp>>(exp.a);
    size_t const32{};
    if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(
         binexp->left) &&
       (const32 =
          std::get<std::unique_ptr<ir::tree::ConstExp>>(binexp->left)->v) <=
         std::numeric_limits<uint32_t>::max()) {
      // MemExp(BinOpExp(ConstExp(const32), reg1, plus)) -> move new_reg, [reg1 + const32]
      auto reg1 = munch_exp(binexp->right);
      auto result = ir::TempGen::new_temp();
      list.emplace_back(::codegen::assem::Oper{
        .assem{std::format("move `d0, [`s0 + {}]", const32)},
        .dst{result},
        .src{reg1},
        .jmp{},
      });
      return result;
    }
    else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(
              binexp->right) &&
            (const32 =
               std::get<std::unique_ptr<ir::tree::ConstExp>>(binexp->right)
                 ->v) <= std::numeric_limits<uint32_t>::max()) {
      // MemExp(BinOpExp(reg1, ConstExp(const32), plus)) -> move new_reg, [reg1 + const32]
      auto reg1 = munch_exp(binexp->left);
      auto result = ir::TempGen::new_temp();
      list.emplace_back(::codegen::assem::Oper{
        .assem{std::format("move `d0, [`s0 + {}]", const32)},
        .dst{result},
        .src{reg1},
        .jmp{},
      });
      return result;
    }

    // MemExp(BinOpExp(reg1, reg2, plus)) -> move new_reg, [reg1 + reg2]
    auto reg1 = munch_exp(binexp->left);
    auto reg2 = munch_exp(binexp->right);
    auto result = ir::TempGen::new_temp();
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, [`s0 + `s1]"},
      .dst{result},
      .src{reg1, reg2},
      .jmp{},
    });
    return result;
  }

  // MemExp(reg1) -> move new_reg, [reg1]
  auto reg1 = munch_exp(exp.a);
  auto result = ir::TempGen::new_temp();
  list.emplace_back(::codegen::assem::Oper{
    .assem{"move `d0, [`s0]"},
    .dst{result},
    .src{reg1},
    .jmp{},
  });
  return result;
}

bool MuxMunchGen::maybe_munch_store(const ir::tree::MoveStmt& stmt)
{
  if(!std::holds_alternative<std::unique_ptr<ir::tree::MemExp>>(stmt.left)) {
    return false;
  }
  auto& memexp = std::get<std::unique_ptr<ir::tree::MemExp>>(stmt.left);

  if(std::holds_alternative<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a) &&
     std::get<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a)->op ==
       ir::tree::BinaryOp::plus) {
    auto& binopexp = std::get<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a);
    // MoveStmt(MemExp(BinOpExp(ConstExp(const32), reg1), plus)), reg2) -> move QWORD PTR [reg1 + const32], reg2
    // MoveStmt(MemExp(BinOpExp(reg1, ConstExp(const32), plus)), reg2)  -> move QWORD PTR [reg1 + const32], reg2
    if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(
         binopexp->left) &&
       is_const32(
         std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left)->v)) {
      auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left)->v;
      auto reg1 = munch_exp(binopexp->right);
      auto reg2 = munch_exp(stmt.right);
      list.emplace_back(::codegen::assem::Oper{
        .assem{std::format("move QWORD PTR [`s0 + {}], `s1", c)},
        .dst{},
        .src{reg1, reg2},
        .jmp{},
      });
    }
    else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(
              binopexp->right) &&
            is_const32(
              std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->right)
                ->v)) {
      auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left)->v;
      auto reg1 = munch_exp(binopexp->left);
      auto reg2 = munch_exp(stmt.right);
      list.emplace_back(::codegen::assem::Oper{
        .assem{std::format("move QWORD PTR [`s0 + {}], `s1", c)},
        .dst{},
        .src{reg1, reg2},
        .jmp{},
      });
    }
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(
            memexp->a) &&
          is_const32(
            std::get<std::unique_ptr<ir::tree::ConstExp>>(memexp->a)->v)) {
    // MoveStmt(MemExp(ConstExp(const32)), reg1) -> move QWORD PTR [const32], reg1
    auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(memexp->a)->v;
    auto reg1 = munch_exp(stmt.right);
    list.emplace_back(::codegen::assem::Oper{
      .assem{std::format("move QWORD PTR [{}], `s0", c)},
      .dst{},
      .src{reg1},
      .jmp{},
    });
  }
  else {
    // MoveStmt(MemExp(reg1), reg2) -> move QWORD PTR [reg1], reg2
    auto reg1 = munch_exp(memexp->a);
    auto reg2 = munch_exp(stmt.right);
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move QWORD PTR [`s0], `s1"},
      .dst{},
      .src{reg1, reg2},
      .jmp{},
    });
  }
  return true;
}

bool MuxMunchGen::maybe_munch_load(const ir::tree::MoveStmt& stmt)
{
  if(!std::holds_alternative<std::unique_ptr<ir::tree::MemExp>>(stmt.right)) {
    return false;
  }
  auto& memexp = std::get<std::unique_ptr<ir::tree::MemExp>>(stmt.right);

  if(std::holds_alternative<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a) &&
     std::get<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a)->op ==
       ir::tree::BinaryOp::plus) {
    auto& binopexp = std::get<std::unique_ptr<ir::tree::BinOpExp>>(memexp->a);
    // MoveStmt(reg1, MemExp(BinOpExp(ConstExp(const32), reg2, plus))) -> move reg1, [reg2 + const32]
    // MoveStmt(reg1, MemExp(BinOpExp(reg2, ConstExp(const32), plus))) -> move reg1, [reg2 + const32]
    if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(
         binopexp->left) &&
       is_const32(
         std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left)->v)) {
      auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left)->v;
      auto reg2 = munch_exp(binopexp->right);
      auto reg1 = munch_exp(stmt.left);
      list.emplace_back(::codegen::assem::Oper{
        .assem{std::format("move `d0, [`s0 + {}]", c)},
        .dst{reg1},
        .src{reg2},
        .jmp{},
      });
    }
    else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(
              binopexp->right) &&
            is_const32(
              std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->right)
                ->v)) {
      auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(binopexp->left)->v;
      auto reg2 = munch_exp(binopexp->left);
      auto reg1 = munch_exp(stmt.left);
      list.emplace_back(::codegen::assem::Oper{
        .assem{std::format("move `d0, [`s0 + {}]", c)},
        .dst{reg1},
        .src{reg2},
        .jmp{},
      });
    }
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::ConstExp>>(
            memexp->a) &&
          is_const32(
            std::get<std::unique_ptr<ir::tree::ConstExp>>(memexp->a)->v)) {
    // MoveStmt(reg1, MemExp(ConstExp(0))) -> xor reg1, reg1
    // MoveStmt(reg1, MemExp(ConstExp(const32))) -> move reg1, [const32]
    auto c = std::get<std::unique_ptr<ir::tree::ConstExp>>(memexp->a)->v;
    auto reg1 = munch_exp(stmt.left);
    if(c == 0) {
      list.emplace_back(::codegen::assem::Oper{
        .assem{"xor `s0 `s0"},
        .dst{reg1},
        .src{reg1},
        .jmp{},
      });
    }
    else {
      list.emplace_back(::codegen::assem::Oper{
        .assem{std::format("move `d0, [{}]", c)},
        .dst{reg1},
        .src{},
        .jmp{},
      });
    }
  }
  else {
    // MoveStmt(reg1, MemExp(reg2)) -> move reg1, [reg2]
    auto reg1 = munch_exp(stmt.left);
    auto reg2 = munch_exp(memexp->a);
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, [`s0]"},
      .dst{reg1},
      .src{reg2},
      .jmp{},
    });
  }
  return true;
}

ir::TempGen::Temp MuxMunchGen::munch_binop_exp(const ir::tree::BinOpExp& exp)
{
  auto left = munch_exp(exp.left);
  auto right = munch_exp(exp.right);
  auto result = ir::TempGen::new_temp();
  if(exp.op == ir::tree::BinaryOp::plus) {
    // BinOpExp(reg1, reg2, plus) -> move new_reg, reg1; add new_reg, reg2
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, `s0"},
      .dst{result},
      .src{left},
      .jmp{},
    });
    list.emplace_back(::codegen::assem::Oper{
      .assem{"add `d0, `s0"},
      .dst{result},
      .src{right},
      .jmp{},
    });
  }
  else if(exp.op == ir::tree::BinaryOp::minus) {
    // BinOpExp(reg1, reg2, plus)  -> move new_reg, reg1; add new_reg, reg2
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, `s0"},
      .dst{result},
      .src{left},
      .jmp{},
    });
    list.emplace_back(::codegen::assem::Oper{
      .assem{"sub `d0, `s0"},
      .dst{result},
      .src{right},
      .jmp{},
    });
  }
  else if(exp.op == ir::tree::BinaryOp::mul) {
    // BinOpExp(reg1, reg2, mul) -> move rax, reg1; imul reg2; move new_reg, rax
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, `s0"},
      .dst{arch::Frame::RAX},
      .src{left},
      .jmp{},
    });
    list.emplace_back(::codegen::assem::Oper{
      .assem{"imul `s0"},
      .dst{arch::Frame::RAX, arch::Frame::RDX},
      .src{right},
      .jmp{},
    });
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, `s0"},
      .dst{result},
      .src{arch::Frame::RAX},
      .jmp{},
    });
  }
  else if(exp.op == ir::tree::BinaryOp::div) {
    // BinOpExp(reg1, reg2, div) -> move rax, reg1; idiv reg2; move new_reg, rax
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, `s0"},
      .dst{arch::Frame::RAX},
      .src{left},
      .jmp{},
    });
    list.emplace_back(::codegen::assem::Oper{
      .assem{"idiv `s0"},
      .dst{arch::Frame::RAX, arch::Frame::RDX},
      .src{right},
      .jmp{},
    });
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, `s0"},
      .dst{result},
      .src{arch::Frame::RAX},
      .jmp{},
    });
  }
  else {
    // there are other binops that should be handled, but our frontend will not generate them
    assert(false);
  }
  return result;
}

void MuxMunchGen::munch_call_exp(const ir::tree::CallExp& exp)
{
  std::vector<ir::TempGen::Temp> args;
  for(auto& arg : exp.args) {
    args.push_back(munch_exp(arg));
  }
  auto trashed =
    std::vector(std::views::keys(arch::Frame::special_regs).begin(),
                std::views::keys(arch::Frame::special_regs).end());
  trashed.insert(trashed.end(),
                 std::views::keys(arch::Frame::caller_saved).begin(),
                 std::views::keys(arch::Frame::caller_saved).end());

  if(std::holds_alternative<std::unique_ptr<ir::tree::NameExp>>(exp.fun)) {
    // CallExp(NameExp(label),  reg_list) -> call rel32 label
    auto ljmp = std::get<std::unique_ptr<ir::tree::NameExp>>(exp.fun)->label;
    list.emplace_back(::codegen::assem::Oper{
      .assem = std::format("call rel32 {}", ljmp.str()),
      .dst{std::move(trashed)},
      .src{std::move(args)},
      .jmp{},
    });
  }

  // CallExp(reg1, reg_list) -> call reg1
  list.emplace_back(::codegen::assem::Oper{.assem{"call `s0"},
                                           .dst{std::move(trashed)},
                                           .src{std::move(args)},
                                           .jmp{}});
}

void MuxMunchGen::munch_stmt(const ir::tree::Stmt& stmt)
{
  if(std::holds_alternative<std::unique_ptr<ir::tree::MoveStmt>>(stmt)) {
    munch_move_stmt(*std::get<std::unique_ptr<ir::tree::MoveStmt>>(stmt));
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::ExpStmt>>(stmt)) {
    auto& exp = std::get<std::unique_ptr<ir::tree::ExpStmt>>(stmt)->exp;
    if(std::holds_alternative<std::unique_ptr<ir::tree::CallExp>>(exp)) {
      // ExpStmt(CallExp(NameExp(label), reg_list))
      // ExpStmt(CallExp(reg, reg_list))
      munch_call_exp(*std::get<std::unique_ptr<ir::tree::CallExp>>(exp));
    }
    else {
      // ExpStmt(reg) ->
      // discard the result
      (void)munch_exp(exp);
    }
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::JumpStmt>>(stmt)) {
    // JumpStmt(NameExp(label))  -> jmp rel32 label
    auto& jmp = std::get<std::unique_ptr<ir::tree::JumpStmt>>(stmt)->a;
    assert(std::holds_alternative<std::unique_ptr<ir::tree::NameExp>>(jmp));
    auto ljmp = std::get<std::unique_ptr<ir::tree::NameExp>>(jmp)->label;
    list.emplace_back(::codegen::assem::Oper{
      .assem = std::format("jmp rel32 {}", ljmp.str()),
      .dst{},
      .src{},
      .jmp{std::vector{ljmp}},
    });
  }
  else if(std::holds_alternative<std::unique_ptr<ir::tree::CJumpStmt>>(stmt)) {
    munch_cjump_stmt(*std::get<std::unique_ptr<ir::tree::CJumpStmt>>(stmt));
  }

  assert(std::holds_alternative<std::unique_ptr<ir::tree::LabelStmt>>(stmt));
  // LabelStmt(label) -> label:
  auto lab = std::get<std::unique_ptr<ir::tree::LabelStmt>>(stmt)->label;
  list.emplace_back(::codegen::assem::Oper{
    .assem{std::format("{}:", lab.str())},
    .dst{},
    .src{},
    .jmp{},
  });
}

void MuxMunchGen::munch_move_stmt(const ir::tree::MoveStmt& stmt)
{
  if(std::holds_alternative<std::unique_ptr<ir::tree::TempExp>>(stmt.left) &&
     std::holds_alternative<std::unique_ptr<ir::tree::CallExp>>(stmt.right)) {
    // MoveStmt(TempExp(temp), CallExp(NameExp(label), reg_list)) -> call rel32 label; move temp, rax
    // MoveStmt(TempExp(temp), CallExp(reg, reg_list)) -> call reg; move temp, rax
    auto temp = std::get<std::unique_ptr<ir::tree::TempExp>>(stmt.left)->temp;
    auto& callexp = std::get<std::unique_ptr<ir::tree::CallExp>>(stmt.right);
    munch_call_exp(*callexp);
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, `s0"},
      .dst{temp},
      .src{arch::Frame::RAX},
      .jmp{},
    });
  }
  else if(!maybe_munch_store(stmt) && !maybe_munch_load(stmt)) {
    // MoveStmt(reg1, reg2) -> move reg1, reg2
    auto reg1 = munch_exp(stmt.left);
    auto reg2 = munch_exp(stmt.right);
    list.emplace_back(::codegen::assem::Oper{
      .assem{"move `d0, `s0"},
      .dst{reg1},
      .src{reg2},
      .jmp{},
    });
  }
}

void MuxMunchGen::munch_cjump_stmt(const ir::tree::CJumpStmt& stmt)
{
  /*
  CJumpStmt(eq, reg1, reg2, tlab, flab) -> cmp reg1, reg2; je tlab
  CJumpStmt(ne, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jne tlab
  CJumpStmt(lt, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jl tlab
  CJumpStmt(gt, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jg tlab
  CJumpStmt(le, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jle tlab
  CJumpStmt(ge, reg1, reg2, tlab, flab) -> cmp reg1, reg2; jge tlab
  */

  auto reg1 = munch_exp(stmt.lexp);
  auto reg2 = munch_exp(stmt.rexp);

  list.emplace_back(::codegen::assem::Oper{
    .assem{"cmp `s0, `s1"},
    .dst{},
    .src{reg1, reg2},
    .jmp{},
  });

  std::string assem{};
  switch(stmt.op) {
  case ir::tree::RelOp::eq: {
    assem = std::format("je {}", stmt.tlabel.str());
    break;
  }
  case ir::tree::RelOp::ne: {
    assem = std::format("jne {}", stmt.tlabel.str());
    break;
  }
  case ir::tree::RelOp::lt: {
    assem = std::format("jl {}", stmt.tlabel.str());
    break;
  }
  case ir::tree::RelOp::gt: {
    assem = std::format("jg {}", stmt.tlabel.str());
    break;
  }
  case ir::tree::RelOp::le: {
    assem = std::format("jle {}", stmt.tlabel.str());
    break;
  }
  case ir::tree::RelOp::ge: {
    assem = std::format("jge {}", stmt.tlabel.str());
    break;
  }
  default:
    assert(false);
  }

  list.emplace_back(::codegen::assem::Oper{
    .assem{assem},
    .dst{},
    .src{},
    .jmp{std::vector{stmt.tlabel, stmt.flabel}},
  });
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