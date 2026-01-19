#include <arch/x86_64/frame.hpp>
#include <assert.h>

namespace arch2
{

static auto FP = ir::TempGen::new_temp();
static auto RV = ir::TempGen::new_temp();
static auto SP = ir::TempGen::new_temp();
static auto RAX = RV;
static auto RBX = ir::TempGen::new_temp();

static auto RDI = ir::TempGen::new_temp();
static auto RSI = ir::TempGen::new_temp();
static auto RDX = ir::TempGen::new_temp();
static auto RCX = ir::TempGen::new_temp();
static auto R8 = ir::TempGen::new_temp();
static auto R9 = ir::TempGen::new_temp();
static auto R10 = ir::TempGen::new_temp();
static auto R11 = ir::TempGen::new_temp();
static auto R12 = ir::TempGen::new_temp();
static auto R13 = ir::TempGen::new_temp();
static auto R14 = ir::TempGen::new_temp();
static auto R15 = ir::TempGen::new_temp();

// clang-format off
static const std::unordered_map<ir::TempGen::Temp, X86FrameImpl::register_t> reg_mapper{
  {FP, "rbp"},
  {RAX, "rax"},
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
  {R15, "r15"}};
// clang-format on

static const size_t no_registers = reg_mapper.size();
static const std::vector<ir::TempGen::Temp> special_regs{FP, RV, SP};
static const std::vector<ir::TempGen::Temp> caller_saved{RDI, RSI, RCX, RDX, R8, R9, R10, R11};
static const std::vector<ir::TempGen::Temp> callee_saved{RBX, R12, R13, R14, R15};
static const std::vector<ir::TempGen::Temp> params_on_regs{RDI, RSI, RDX, RCX, R8, R9};
static constexpr uint8_t word_size = 8;

std::vector<std::reference_wrapper<const Frame::Access>> X86FrameImpl::formals() const
{
  std::vector<std::reference_wrapper<const Frame::Access>> refs;
  refs.reserve(formals_.size());
  for(const auto& a : formals_)
  {
    refs.push_back(std::ref(a));
  }
  return refs;
}

ir::TempGen::Temp X86FrameImpl::frame_pointer() const
{
  return FP;
};

ir::TempGen::Label X86FrameImpl::name() const
{
  return label;
}

std::unordered_map<ir::TempGen::Temp, X86FrameImpl::register_t>
X86FrameImpl::get_register_mapping() const
{
  return reg_mapper;
};

size_t X86FrameImpl::number_of_registers() const
{
  return no_registers;
}

void X86FrameImpl::rewrite_program(std::list<codegen::assem::Instruction>& list,
                                   const std::unordered_set<ir::TempGen::Temp>& spilled_temps)
{
  // TODO
  static_cast<void>(list);
  static_cast<void>(spilled_temps);
};

ir::Ex X86FrameImpl::exp(const Frame::Access& fax, ir::Ex&& fp) const
{
  auto ax = static_cast<const X86FrameAccessImpl&>(fax).ax;
  // translate an access into an exp
  if(std::holds_alternative<InFrame>(ax))
  {
    auto at = std::make_unique<ir::tree::BinOpExp>(
      ir::tree::BinaryOp::plus,
      std::move(fp),
      std::make_unique<ir::tree::ConstExp>(std::get<InFrame>(ax).offset));
    return std::make_unique<ir::tree::MemExp>(std::move(at));
  }
  else
  {
    return std::make_unique<ir::tree::TempExp>(std::get<InReg>(ax).t);
  }
  assert(false);
  std::unreachable();
}

Frame::Access& X86FrameImpl::alloc_local(bool escape)
{
  locals++;
  if(escape)
  {
    locals_stack_offset -= word_size;
    locals_.push_back(X86FrameAccessImpl{InFrame(locals_stack_offset)});
  }
  else
  {
    locals_.push_back(X86FrameAccessImpl{InReg(ir::TempGen::new_temp())});
  }
  return locals_.back();
}

size_t X86FrameImpl::locals_count() const
{
  return locals_.size();
}

X86FrameImpl::X86FrameImpl(ir::TempGen::Label label, const std::vector<bool>& formals)
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
      auto& ax = X86FrameImpl::alloc_local(true);
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
      formals_.push_back(X86FrameAccessImpl{InReg(temp)});
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
    formals_.push_back(X86FrameAccessImpl{InFrame(off)});
    off += word_size;
  }
}

} // namespace arch