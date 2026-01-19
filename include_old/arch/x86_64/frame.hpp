#pragma once

#include <frame.hpp>
#include <ir/tree.hpp>
#include <variant>

namespace arch2
{

class X86FrameImpl : public Frame
{
  public:
  using stack_offset_t = int64_t;

  struct InReg
  {
    InReg(ir::TempGen::Temp t)
      : t(t)
    { }
    ir::TempGen::Temp t;
  };

  struct InFrame
  {
    // offset from the frame pointer
    InFrame(stack_offset_t offset)
      : offset(offset)
    { }
    stack_offset_t offset;
  };

  struct X86FrameAccessImpl : public Frame::Access
  {
    X86FrameAccessImpl(std::variant<std::monostate, InReg, InFrame> v)
      : Frame::Access()
      , ax(v)
    { }
    std::variant<std::monostate, InReg, InFrame> ax;
  };

  std::vector<std::reference_wrapper<const Frame::Access>> formals() const override;
  ir::TempGen::Temp frame_pointer() const override;
  ir::TempGen::Label name() const override;
  std::unordered_map<ir::TempGen::Temp, register_t> get_register_mapping() const override;
  size_t number_of_registers() const override;
  void rewrite_program(std::list<codegen::assem::Instruction>& list,
                       const std::unordered_set<ir::TempGen::Temp>& spilled_temps) override;
  ir::Ex exp(const Frame::Access& fax, ir::Ex&& fp) const override;
  Frame::Access& alloc_local(bool escape) override;
  size_t locals_count() const override;
  friend struct X86FrameFactoryImpl;

  private:
  X86FrameImpl(ir::TempGen::Label label, const std::vector<bool>& formals);
  std::vector<Frame::Access> formals_;
  std::vector<Frame::Access> locals_;
  ir::tree::Stmt view_shift{};
  ir::TempGen::Label label;
  stack_offset_t locals_stack_offset{};
  uint32_t locals{};
};

struct X86FrameFactoryImpl : public FrameFactory
{
  std::unique_ptr<Frame> make_frame(ir::TempGen::Label label,
                                    const std::vector<bool>& formals) const override
  {
    return std::unique_ptr<Frame>(new X86FrameImpl(label, formals));
  }
};

} // namespace arch