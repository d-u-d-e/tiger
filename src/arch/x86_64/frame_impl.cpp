#include "arch/x86_64/frame_impl.hpp"
#include "assem.hpp"
#include "temp.hpp"
#include <algorithm>
#include <cassert>
#include <list>
#include <optional>
#include <vector>

namespace arch
{

X86Frame::X86Frame(TempGen::Label label, const std::vector<bool>& formals)
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
      auto temp = TempGen::new_temp();
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
  // so we start at off = 2 * word_size
  stack_offset_t off = 2 * word_size;
  for(size_t i = params_on_regs.size(); i < formals.size(); i++)
  {
    formals_.push_back(InFrame(off));
    off += word_size;
  }
}

std::vector<X86Frame::Access> X86Frame::formals() const
{
  return formals_;
}

uint16_t X86Frame::locals_count() const
{
  return locals;
}

ir::tree::Stmt X86Frame::proc_entry_exit1(ir::tree::Stmt&& stmt)
{
  // proc_entry_exit1 does the following:
  // - mov incoming register formal params to the place expected by the function (view shift)
  // - save callee saved registers
  // - restore callee saved registers

  std::vector<ir::tree::Stmt> save;
  std::vector<ir::tree::Stmt> restore;

  // since the reg allocator implements spilling, callee saved regs are not pushed to the stack
  // but moved to a temporary
  for(auto& reg : callee_saved)
  {
    auto t = TempGen::new_temp();
    save.push_back(std::make_unique<ir::tree::MoveStmt>(std::make_unique<ir::tree::TempExp>(t),
                                                        std::make_unique<ir::tree::TempExp>(reg)));
    restore.push_back(std::make_unique<ir::tree::MoveStmt>(std::make_unique<ir::tree::TempExp>(reg),
                                                           std::make_unique<ir::tree::TempExp>(t)));
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

X86Frame::Access X86Frame::alloc_local(bool escape)
{
  locals++;
  if(escape)
  {
    locals_stack_offset -= word_size;
    return InFrame(locals_stack_offset);
  }
  else
  {
    return InReg(TempGen::new_temp());
  }
}

TempGen::Label X86Frame::name() const
{
  return label;
}

X86Frame::stack_offset_t X86Frame::alloc_spilled_temporary()
{
  spilled_temps++;
  auto next = locals_stack_offset - spilled_temps * word_size;
  return next;
}

ir::Ex X86Frame::exp(const X86Frame::Access& fax, ir::Ex&& fp)
{
  // traslate an access into an exp
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

ir::Ex X86Frame::external_call(TempGen::Label label, std::vector<ir::Ex>&& args)
{
  // external calls on Linux will use the System V abi, so this should be fine
  return std::make_unique<ir::tree::CallExp>(std::make_unique<ir::tree::NameExp>(label),
                                             std::move(args));
}

void X86Frame::proc_entry_exit2(std::list<assem::Instruction>& list)
{
  // proc_entry_exit2 does the following:
  // - find the max number of outgoing parameters for proc_entry_exit3
  // - append a sink instruction for live registers at the end of the procedure call

  uint32_t outgoing_params{};
  for(auto& i : list)
  {
    if(std::holds_alternative<assem::Oper>(i))
    {
      auto& oper = std::get<assem::Oper>(i);
      if(oper.assem.starts_with("*"))
      {
        outgoing_params++;
        max_outgoing_params = std::max(outgoing_params, max_outgoing_params);
      }
      else if(oper.assem.starts_with("call"))
      {
        // we computed all the outgoing parameters
        outgoing_params = 0;
      }
    }
  }

  // append sink instruction
  auto live = std::vector({RAX, SP, FP});
  std::copy(callee_saved.begin(), callee_saved.end(), std::back_inserter(live));
  list.push_back(assem::Oper{.assem{""}, .dst{}, .src{live}, .jmp{}});
}

std::pair<std::string, std::string> X86Frame::proc_entry_exit3(std::list<assem::Instruction>& list)
{
  // proc_entry_exit3 does the following:
  // - patch instructions that allocate stack space for outgoing parameters (see munch_args)
  // - implement the prologue/epilogue

  // stack space is allocated as follows (going downwards):
  // locals
  // spilled temporaries
  // extra alignment for 16 bytes if needed
  // max outgoing params

  // we align down to a multiple of 16 bytes
  // we indirectly save the return address and the old fp for a total of 16 bytes

  auto space =
    (-locals_stack_offset + spilled_temps * word_size + word_size * max_outgoing_params + 15) & ~15;
  stack_offset_t off = -space + word_size * max_outgoing_params;

  // patch instructions
  uint32_t outgoing_param{};
  for(auto& i : list)
  {
    if(std::holds_alternative<assem::Oper>(i))
    {
      auto& oper = std::get<assem::Oper>(i);
      if(oper.assem.starts_with("*"))
      {
        // instruction that need to be patched
        outgoing_param++;
        i = assem::Oper{
          .assem{std::format("mov  QWORD PTR [`s0{}], `s1\n", off - word_size * outgoing_param)},
          .dst{},
          .src{oper.src},
          .jmp{}};
      }
      else if(oper.assem.starts_with("call"))
      {
        outgoing_param = 0;
      }
    }
  }

  std::string prologue;
  if(label.str() == "tiger_main")
  {
    prologue = ".global tiger_main\n";
  }

  prologue += std::format(".type {}, @function\n"
                          "{}:\n"
                          "push rbp\n"
                          "mov  rbp, rsp\n"
                          "sub  rsp, {}\n",
                          label.str(),
                          label.str(),
                          space);

  std::string epilogue = "mov  rsp, rbp\n"
                         "pop  rbp\n"
                         "ret  \n";

  return {prologue, epilogue};
}

void X86Frame::rewrite_program(std::list<assem::Instruction>& list,
                               const std::unordered_set<TempGen::Temp>& spilled_temps)
{
  // TODO: make this more efficient
  // we should take into account that instructions can access memory, so that instead of rewriting
  // something like: add t1, t2 -> mov t, [x]; add t1, t
  // we should simply do: add t1, [x], assuming t2 is spilled at address x
  // and we should not alloc a new frame local for each spilled reg, but instead
  // create enough locals to accomodate all spilled regs
  // (requires an interference graph for spilled temporaries)

  std::unordered_map<TempGen::Temp, stack_offset_t> locations;

  auto get_off = [&locations, this](TempGen::Temp t) {
    stack_offset_t off{};
    if(locations.contains(t))
    {
      off = locations.at(t);
    }
    else
    {
      off = alloc_spilled_temporary();
      locations.emplace(t, off);
    }
    return off;
  };

  auto rewrite = [&spilled_temps, &get_off, &list](std::vector<TempGen::Temp>& src,
                                                   std::vector<TempGen::Temp>& dst,
                                                   std::list<assem::Instruction>::iterator iter) {
    for(auto t : spilled_temps)
    {
      // get the offset of the spilled temporary
      auto off = get_off(t);

      std::optional<TempGen::Temp> new_temp{};

      // look in src
      auto it = std::find(src.begin(), src.end(), t);
      if(it != src.end())
      {
        // temporary occurs in src, replace it with a new one, short-lived
        new_temp = TempGen::new_temp();

        // fetch the new temporary
        list.insert(
          iter,
          assem::Instruction{assem::Oper{.assem = std::format("mov  `d0, [`s0{:+}]\n", off),
                                         .dst{new_temp.value()},
                                         .src{FP},
                                         .jmp{}}});

        // fix the instruction to use the new temporary
        *it = new_temp.value();
      }

      // look in dst
      it = std::find(dst.begin(), dst.end(), t);
      if(it != dst.end())
      {
        // temporary occurs in dst, use previous if it exists
        if(!new_temp.has_value())
        {
          new_temp = TempGen::new_temp();
        }

        // store the new temporary
        list.insert(std::next(iter),
                    assem::Instruction{
                      assem::Oper{.assem = std::format("mov  QWORD PTR [`s0{:+}], `s1\n", off),
                                  .dst{},
                                  .src{FP, new_temp.value()},
                                  .jmp{}}});

        // fix the instruction to use the new temporary
        *it = new_temp.value();
      }
    }
  };

  auto rewrite_move = [&spilled_temps, &get_off, &list](
                        TempGen::Temp& src,
                        TempGen::Temp& dst,
                        std::list<assem::Instruction>::iterator iter) {
    auto dst_spilled = spilled_temps.contains(dst);
    auto src_spilled = spilled_temps.contains(src);

    if(!dst_spilled && !src_spilled)
    {
      return;
    }
    else if(dst_spilled && !src_spilled)
    {
      *iter = assem::Oper{.assem = std::format("mov  QWORD PTR [`s0{:+}], `s1\n", get_off(dst)),
                          .dst = {},
                          .src = {FP, src},
                          .jmp = {}};
    }
    else if(!dst_spilled && src_spilled)
    {
      *iter = assem::Oper{.assem = std::format("mov  `d0, [`s0{:+}]\n", get_off(src)),
                          .dst = {dst},
                          .src = {FP},
                          .jmp = {}};
    }
    else
    {
      // both spilled
      auto new_t = TempGen::new_temp();
      *iter = assem::Oper{.assem = std::format("mov  `d0, [`s0{:+}]\n", get_off(src)),
                          .dst = {new_t},
                          .src = {FP},
                          .jmp = {}};
      list.insert(std::next(iter),
                  assem::Oper{.assem = std::format("mov  QWORD PTR [`s0{:+}], `s1\n", get_off(dst)),
                              .dst = {},
                              .src = {FP, new_t},
                              .jmp = {}});
    }
  };

  for(auto iter = list.begin(); iter != list.end();)
  {
    auto& i = *iter;
    auto next_iter = std::next(iter);
    // rewrite may change the list by inserting instructions before the current iter,
    // or after the current iterator, so we save the true next instruction

    if(std::holds_alternative<assem::Move>(i))
    {
      auto& move = std::get<assem::Move>(i);
      rewrite_move(move.src, move.dst, iter);
    }
    else if(std::holds_alternative<assem::Oper>(i))
    {
      auto& oper = std::get<assem::Oper>(i);
      rewrite(oper.src, oper.dst, iter);
    }
    iter = next_iter;
  }
}

} // namespace arch