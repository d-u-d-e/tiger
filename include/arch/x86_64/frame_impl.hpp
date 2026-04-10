#pragma once
#include "assem.hpp"
#include "ir/fragment.hpp"
#include "ir/tree.hpp"
#include "temp.hpp"
#include <list>
#include <unordered_map>
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
  using esc_offset_t = uint64_t;
  using Temp = TempGen::Temp;
  using Label = TempGen::Label;

  static inline auto RV = TempGen::new_temp();
  static constexpr uint8_t word_size = 8;

  struct InReg
  {
    explicit InReg(Temp t)
      : t(t)
    { }
    Temp t;
  };

  struct InFrame
  {
    // offset from the frame pointer
    explicit InFrame(stack_offset_t offset)
      : offset(offset)
    { }
    stack_offset_t offset;
  };

  struct InEscapingRecord
  {
    // offset from the escaping record
    explicit InEscapingRecord(esc_offset_t offset)
      : offset(offset)
    { }
    esc_offset_t offset;
  };

  using Access = std::variant<std::monostate, InReg, InFrame, InEscapingRecord>;

  X86Frame(Label label, const std::vector<bool>& formals);
  X86Frame(const X86Frame&) = delete;
  auto operator=(const X86Frame&) -> X86Frame& = delete;
  X86Frame(X86Frame&&) = delete;
  auto operator=(X86Frame&&) -> X86Frame& = delete;
  ~X86Frame() = default;

  auto formals() const -> std::vector<Access>;
  auto name() const -> TempGen::Label;

  auto proc_entry_exit1(ir::tree::Stmt&& stmt) -> ir::tree::Stmt;
  void proc_entry_exit2(std::list<assem::Instruction>& list);
  auto proc_entry_exit3(std::list<assem::Instruction>& list) -> std::pair<std::string, std::string>;
  void rewrite_program(std::list<assem::Instruction>& list,
                       const std::unordered_set<TempGen::Temp>& spilled_temps);
  auto alloc_local(bool escape) -> Access;

  static auto exp(const Access& fax, ir::Ex&& ep) -> ir::Ex;
  static auto external_call(const TempGen::Label& label, std::vector<ir::Ex>&& args) -> ir::Ex;
  static auto assembler_directives_begin() -> std::string
  {
    return ".intel_syntax noprefix\n";
  }
  static auto assembler_directives_end() -> std::string
  {
    return ".section .note.GNU-stack,\"\",@progbits\n";
  }
  static auto emit_string(const ir::StringFragment& f) -> std::string
  {
    return std::format("{}:\n"
                       ".asciz \"{}\"\n",
                       f.label.str(),
                       f.lit);
  }
  static auto get_temporary_register_mapping()
    -> const std::unordered_map<TempGen::Temp, assem::register_t>&
  {
    return temp_map;
  }

  auto escaping_pointer() const -> TempGen::Temp
  {
    return EP;
  }

  private:
  auto alloc_spilled_temporary() -> stack_offset_t;
  auto alloc_escaping_record() const -> ir::Nx;

  static inline auto FP = TempGen::new_temp();
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

  const TempGen::Temp EP;

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
  std::unordered_map<TempGen::Temp, X86Frame::Access> escaping_formals;
  uint32_t max_outgoing_params{};
  uint32_t spilled_temps{};
  esc_offset_t escaping_offset{};
  uint32_t escaping_locals{};
};
} // namespace arch