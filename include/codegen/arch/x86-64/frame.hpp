#pragma once
#include <algorithm>
#include <bits/ranges_algo.h>
#include <cassert>
#include <codegen/assem.hpp>
#include <cstddef>
#include <cstdint>
#include <format>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace arch
{

class Frame {
  public:
  private:
  struct InReg {
    InReg(ir::TempGen::Temp t)
      : t(t)
    { }
    ir::TempGen::Temp t;
  };

  struct InFrame {
    // offset from the frame pointer
    InFrame(int16_t offset)
      : offset(offset)
    { }
    int16_t offset;
  };

  public:
  using Access = std::variant<std::monostate, InReg, InFrame>;
  static inline constexpr uint8_t word_size = 8;

  /* 
    System V calling convention
    callee-saved: rbx, rbp, rsp, r12, r13, r14, r15
    caller-saved: rax, rdi, rsi, rcx, rdx, r8, r9, r10, r11
    Parameters to functions are passed in via the registers rdi, rsi, rdx, rcx, r8, and r9.
    Any additional arguments that do not fit in these registers are passed on the stack in reverse order. 
    Parameters passed via the stack may be modified by the called function.
    The return value is stored in the rax register.
    Functions are called using the call instruction, which pushes the address of the next instruction onto the stack and 
    jumps to the operand. 
    Functions return to the caller using the ret instruction, which pops the return address from the stack and jumps to it. 
    The stack is 16-byte aligned just before the call instruction is executed.
  */

  using register_t = std::string;
  static inline auto FP = ir::TempGen::new_temp();
  static inline auto RV = ir::TempGen::new_temp();
  static inline auto SP = ir::TempGen::new_temp();
  static inline auto RAX = RV;
  static inline auto RBX = ir::TempGen::new_temp();

  static inline auto RDI = ir::TempGen::new_temp();
  static inline auto RSI = ir::TempGen::new_temp();
  static inline auto RDX = ir::TempGen::new_temp();
  static inline auto RCX = ir::TempGen::new_temp();
  static inline auto R8 = ir::TempGen::new_temp();
  static inline auto R9 = ir::TempGen::new_temp();
  static inline auto R10 = ir::TempGen::new_temp();
  static inline auto R11 = ir::TempGen::new_temp();
  static inline auto R12 = ir::TempGen::new_temp();
  static inline auto R13 = ir::TempGen::new_temp();
  static inline auto R14 = ir::TempGen::new_temp();
  static inline auto R15 = ir::TempGen::new_temp();

  // clang-format off
  static inline std::unordered_map<ir::TempGen::Temp, register_t> temp_map{
    {FP, "rbp"},
    {RV, "rax"},
    {SP, "rsp"},
    {RDI, "rdi"},
    {RSI, "rsi"},
    {RCX, "rcx"},
    {RDX, "rdx"},
    {R8, "r8"},
    {R9, "r9"},
    {R10, "r10"},
    {R11, "r11"},
    {RBX, "rbx"},
    {R12, "r12"},
    {R13, "r13"},
    {R14, "r14"},
    {R15, "r15"}
  };
  // clang-format on

  static inline std::vector<ir::TempGen::Temp> special_regs{FP, RV, SP};
  static inline std::vector<ir::TempGen::Temp> caller_saved{RDI, RSI, RCX, RDX, R8, R9, R10, R11};
  static inline std::vector<ir::TempGen::Temp> callee_saved{RBX, R12, R13, R14, R15};
  static inline std::vector<ir::TempGen::Temp> params_on_regs{RDI, RSI, RDX, RCX, R8, R9};

  Frame(ir::TempGen::Label label, const std::vector<bool>& formals)
    : label(label)
  {
    // just a dummy stmt
    view_shift = std::make_unique<ir::tree::ExpStmt>(std::make_unique<ir::tree::ConstExp>(0));

    for(size_t i = 0; i < std::min(params_on_regs.size(), formals.size()); i++)
    {
      auto& reg = params_on_regs[i];
      if(formals[i])
      {
        // param escapes, but is passed on a register
        auto ax = alloc_local(true);
        formals_.push_back(ax);
        // generate a mov stmt to the stack location
        view_shift = std::make_unique<ir::tree::SeqStmt>(
          std::move(view_shift),
          std::make_unique<ir::tree::MoveStmt>(exp(ax, std::make_unique<ir::tree::TempExp>(FP)),
                                               std::make_unique<ir::tree::TempExp>(reg)));
      }
      else
      {
        auto temp = ir::TempGen::new_temp();
        formals_.push_back(InReg(temp));
        // generate a mov stmt to the fresh temp
        view_shift = std::make_unique<ir::tree::SeqStmt>(
          std::move(view_shift),
          std::make_unique<ir::tree::MoveStmt>(std::make_unique<ir::tree::TempExp>(temp),
                                               std::make_unique<ir::tree::TempExp>(reg)));
      }
    }

    // the remaining params are passed on the stack, but recall that with respect to the
    // current fp, we need to go past the saved fp and the return address which are on the stack
    // so we start at off = 3 * word_size
    int16_t off = 3 * word_size;
    for(size_t i = params_on_regs.size(); i < formals.size(); i++)
    {
      formals_.push_back(InFrame(off));
      off += word_size;
    }
  }

  ir::tree::Stmt proc_entry_exit1(ir::tree::Stmt&& stmt)
  {
    // proc_entry_exit1 does the following:
    // - mov incoming register formal params to the place expected by the function
    // - save callee saved registers
    // - restore callee saved registers
    // callee saved regs should be saved to the frame depending whether the reg allocator implements spilling

    // TODO: review when spilling is implemented
    std::vector<ir::tree::Stmt> save;
    std::vector<ir::tree::Stmt> restore;
    for(auto& reg : callee_saved)
    {
      auto ax = alloc_local(true);
      save.push_back(
        std::make_unique<ir::tree::MoveStmt>(exp(ax, std::make_unique<ir::tree::TempExp>(FP)),
                                             std::make_unique<ir::tree::TempExp>(reg)));
      restore.push_back(
        std::make_unique<ir::tree::MoveStmt>(std::make_unique<ir::tree::TempExp>(reg),
                                             exp(ax, std::make_unique<ir::tree::TempExp>(FP))));
    }
    auto folder = [](auto&& arg1, auto&& arg2) {
      return ir::tree::Stmt(std::make_unique<ir::tree::SeqStmt>(std::move(arg1), std::move(arg2)));
    };

    auto save_seq = std::ranges::fold_left_first(
      std::make_move_iterator(save.begin()), std::make_move_iterator(save.end()), folder);

    auto restore_seq = std::ranges::fold_left_first(
      std::make_move_iterator(restore.begin()), std::make_move_iterator(restore.end()), folder);

    assert(save_seq.has_value());
    assert(restore_seq.has_value());

    // view-shift -> save sequence -> body -> restore sequence
    stmt = std::make_unique<ir::tree::SeqStmt>(std::move(save_seq.value()), std::move(stmt));
    stmt = std::make_unique<ir::tree::SeqStmt>(std::move(stmt), std::move(restore_seq.value()));
    stmt = std::make_unique<ir::tree::SeqStmt>(std::move(view_shift), std::move(stmt));
    return stmt;
  }

  void proc_entry_exit2(std::vector<::codegen::assem::Instruction>& list)
  {
    // proc_entry_exit2 does the following:
    // - append a sink instruction to the body to tell the register allocator that certain regs are live at procedure exit
    // - patch instructions that allocate stack space for outgoing parameters (see munch_args)

    // we will need to make the stack 16-byte aligned just before the CALL instruction
    uint16_t outgoing_params{};
    for(auto& i : list)
    {
      if(std::holds_alternative<::codegen::assem::Oper>(i))
      {
        auto& oper = std::get<::codegen::assem::Oper>(i);
        if(oper.assem.starts_with("*"))
        {
          // instruction that need to be patched
          outgoing_params++;
          if(outgoing_params > max_outgoing_params)
          {
            max_outgoing_params = outgoing_params;
          }
          i = ::codegen::assem::Oper{
            .assem{std::format("mov  [`s0{}], `s1\n",
                               locals_stack_offset - word_size * outgoing_params)},
            .dst{},
            .src{SP, oper.src[0]},
            .jmp{}};
        }
        else if(oper.assem.starts_with("call"))
        {
          // we computed all the outgoing parameters
          outgoing_params = 0;
        }
      }
    }

    // append sink instruction (is this enough? TODO)
    auto live = std::vector({arch::Frame::RAX, arch::Frame::SP, arch::Frame::FP});
    std::copy(callee_saved.begin(), callee_saved.end(), std::back_inserter(live));
    list.push_back(::codegen::assem::Oper{.assem{""}, .dst{}, .src{live}, .jmp{}});
  }

  std::pair<std::string, std::string>
  proc_entry_exit3(std::vector<::codegen::assem::Instruction>& list)
  {
    // proc_entry_exit3 does the following:
    // - implement the prologue/epilogue

    // stack space is allocated as follows (going downwards):
    // locals
    // max outgoing params

    (void)list; // actually not used
    uint16_t space = -locals_stack_offset + max_outgoing_params * word_size;
    // let's align the stack on a 16 byte boundary, keeping in mind that we also save indirectly
    // the return address and the old fp (2 * word_size == 16)
    space = (space + 15) & ~15;

    std::string prologue = std::format(".type {}, @function\n"
                                       "{}:\n"
                                       "push rbp\n"
                                       "mov  rbp, rsp\n"
                                       "sub  rsp, {}\n",
                                       label.str(),
                                       label.str(),
                                       space);

    std::string epilogue = "mov  rsp, rpb\n"
                           "pop  rbp\n"
                           "ret  \n";

    return {prologue, epilogue};
  }

  const std::vector<Access>& formals() const
  {
    return formals_;
  }

  ir::TempGen::Label name() const
  {
    return label;
  }

  static std::optional<register_t> map_temp(const ir::TempGen::Temp& t)
  {
    if(temp_map.find(t) != temp_map.end())
    {
      return temp_map[t];
    }
    return std::nullopt;
  }

  Access alloc_local(bool escape)
  {
    locals++;
    if(escape)
    { 
      assert(locals_stack_offset - word_size < locals_stack_offset); // overflow
      locals_stack_offset -= word_size;
      return InFrame(locals_stack_offset);
    }
    else
    {
      return InReg(ir::TempGen::new_temp());
    }
  }

  uint16_t locals_count() const
  {
    return locals;
  }

  static std::string to_string(const Access& ax)
  {
    if(std::holds_alternative<InReg>(ax))
    {
      return std::format("InReg(t{})", std::get<InReg>(ax).t);
    }
    else
    {
      return std::format("InFrame({})", std::get<InFrame>(ax).offset);
    }
  }

  static ir::Ex exp(const Access& fax, ir::Ex&& fp)
  {
    // translate an access into an exp
    if(std::holds_alternative<InFrame>(fax))
    {
      auto at = std::make_unique<ir::tree::BinOpExp>(
        ir::tree::BinaryOp::plus,
        std::move(fp),
        std::make_unique<ir::tree::ConstExp>(std::get<InFrame>(fax).offset));
      return std::make_unique<ir::tree::MemExp>(std::move(at));
    }
    else
    {
      return std::make_unique<ir::tree::TempExp>(std::get<InReg>(fax).t);
    }
    assert(false);
  }

  static ir::Ex external_call(ir::TempGen::Label label, std::vector<ir::Ex>&& args)
  {
    // external calls on Linux will use the System V abi, so this should be fine
    return std::make_unique<ir::tree::CallExp>(std::make_unique<ir::tree::NameExp>(label),
                                               std::move(args));
  }

  ir::tree::Stmt view_shift{};
  uint16_t max_outgoing_params{};

  private:
  int16_t locals_stack_offset{};
  std::vector<Access> formals_;
  ir::TempGen::Label label;
  uint16_t locals{};
};

} // namespace arch