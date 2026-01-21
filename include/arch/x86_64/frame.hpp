#pragma once
#include "assem.hpp"
#include "ir/fragment.hpp"
#include "ir/tree.hpp"
#include "temp.hpp"
#include <list>
#include <unordered_set>
#include <variant>
#include <vector>

namespace arch
{
class X86Frame
{
  public:
  friend class X86Generator;

  using stack_offset_t = int64_t;
  using Temp = TempGen::Temp;
  using Label = TempGen::Label;

  static inline auto FP = TempGen::new_temp();
  static inline auto RV = TempGen::new_temp();

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
  void proc_entry_exit2(std::list<assem::Instruction>& list);
  std::pair<std::string, std::string> proc_entry_exit3(std::list<assem::Instruction>& list);
  uint16_t locals_count() const;
  static ir::Ex exp(const Access& fax, ir::Ex&& fp);
  static ir::Ex external_call(TempGen::Label label, std::vector<ir::Ex>&& args);
  void rewrite_program(std::list<assem::Instruction>& list,
                       const std::unordered_set<TempGen::Temp>& spilled_temps);
  TempGen::Label name() const;
  Access alloc_local(bool escape);
  static inline constexpr uint8_t word_size = 8;
  static std::string assembler_directives_begin()
  {
    return ".intel_syntax noprefix\n";
  }
  static std::string assembler_directives_end()
  {
    return ".section .note.GNU-stack,\"\",@progbits\n";
  }
  static std::string emit_string(const ir::StringFragment& f)
  {
    return std::format("{}:\n"
                       ".asciz \"{}\"\n",
                       f.label.str(),
                       f.lit);
  }
  static std::optional<assem::register_t> map_temp(const TempGen::Temp& t);
  static const std::unordered_map<TempGen::Temp, assem::register_t>&
  get_temporary_register_mapping()
  {
    return temp_map;
  }

  private:
  stack_offset_t alloc_spilled_temporary();

  private:
  static inline auto SP = TempGen::new_temp();
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

  // clang-format off
  static inline std::unordered_map<TempGen::Temp, assem::register_t> temp_map{
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

  Label label;
  ir::tree::Stmt view_shift{};
  std::vector<Access> formals_;
  uint32_t locals{};
  stack_offset_t locals_stack_offset{};
  uint32_t max_outgoing_params{};
  uint32_t spilled_temps{};
};
} // namespace arch