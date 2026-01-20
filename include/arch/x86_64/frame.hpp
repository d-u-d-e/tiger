#pragma once
#include "ir/tree.hpp"
#include "temp.hpp"
#include <variant>
#include <vector>

namespace arch
{
class X86Frame
{
  public:
  using stack_offset_t = int64_t;
  using Temp = TempGen::Temp;
  using Label = TempGen::Label;
  using register_t = std::string;

  static inline auto FP = TempGen::new_temp();
  static inline auto RV = TempGen::new_temp();
  static inline auto SP = TempGen::new_temp();

  struct InReg
  {
    InReg(Temp t)
      : t(t)
    { }
    Temp t;
  };

  struct InFrame
  {
    // offset from the frame pointer
    InFrame(stack_offset_t offset)
      : offset(offset)
    { }
    stack_offset_t offset;
  };

  using Access = std::variant<std::monostate, InReg, InFrame>;

  X86Frame(Label label, const std::vector<bool>& formals);
  std::vector<Access> formals() const;
  ir::tree::Stmt proc_entry_exit1(ir::tree::Stmt&& stmt);

  private:
  static inline auto RAX = RV;
  static inline auto RBX = TempGen::new_temp();
  static inline auto RDI = TempGen::new_temp();
  static inline auto RSI = TempGen::new_temp();
  static inline auto RDX = TempGen::new_temp();
  static inline auto RCX = TempGen::new_temp();
  static inline auto R8 = TempGen::new_temp();
  static inline auto R9 = TempGen::new_temp();
  static inline auto R10 = TempGen::new_temp();
  static inline auto R11 = TempGen::new_temp();
  static inline auto R12 = TempGen::new_temp();
  static inline auto R13 = TempGen::new_temp();
  static inline auto R14 = TempGen::new_temp();
  static inline auto R15 = TempGen::new_temp();

  static inline std::vector<TempGen::Temp> special_regs{FP, RV, SP};
  static inline std::vector<TempGen::Temp> caller_saved{RDI, RSI, RCX, RDX, R8, R9, R10, R11};
  static inline std::vector<Temp> callee_saved{RBX, R12, R13, R14, R15};
  static inline std::vector<Temp> params_on_regs{RDI, RSI, RDX, RCX, R8, R9};

  Label label;
  ir::tree::Stmt view_shift{};
  std::vector<Access> formals_;
};
} // namespace arch