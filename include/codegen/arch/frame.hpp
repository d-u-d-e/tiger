#pragma once
#include <cassert>
#include <cstdint>
#include <format>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <memory>
#include <string>
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

  // cdecl calling convention
  // rax, rcx, and rdx are caller-saved, and the rest are callee-saved
  static inline auto FP = ir::TempGen::new_temp();
  static inline auto RV = ir::TempGen::new_temp();
  static inline auto SP = ir::TempGen::new_temp();

  static inline std::unordered_map<ir::TempGen::Temp, std::string> special_regs{
    {FP, "rbp"}, {RV, "rax"}, {SP, "rsp"}};

  static inline std::unordered_map<ir::TempGen::Temp, std::string> caller_saved{
    {ir::TempGen::new_temp(), "rcx"}, {ir::TempGen::new_temp(), "rdx"}};

  static inline std::unordered_map<ir::TempGen::Temp, std::string> callee_saved{
    {ir::TempGen::new_temp(), "rbx"},
    {ir::TempGen::new_temp(), "rsi"},
    {ir::TempGen::new_temp(), "rdi"},
    {ir::TempGen::new_temp(), "r8"},
    {ir::TempGen::new_temp(), "r9"},
    {ir::TempGen::new_temp(), "r10"},
    {ir::TempGen::new_temp(), "r11"},
    {ir::TempGen::new_temp(), "r12"},
    {ir::TempGen::new_temp(), "r13"},
    {ir::TempGen::new_temp(), "r14"},
    {ir::TempGen::new_temp(), "r15"}};

  Frame(ir::TempGen::Label label, const std::vector<bool>& formals)
    : label(label)
  {
    for(auto escape : formals) {
      int16_t off = word_size;
      if(escape) {
        formals_.push_back(InFrame(off));
        off += word_size; // incoming params
      }
      else {
        formals_.push_back(InReg(ir::TempGen::new_temp()));
      }
    }
  }

  const std::vector<Access>& formals() const
  {
    return formals_;
  }

  ir::TempGen::Label name() const
  {
    return label;
  }

  static std::optional<std::string> map_temp(const ir::TempGen::Temp& t)
  {
    if(auto i = special_regs.find(t); i != special_regs.end()) {
      return i->second;
    }
    if(auto i = callee_saved.find(t); i != callee_saved.end()) {
      return i->second;
    }
    if(auto i = caller_saved.find(t); i != caller_saved.end()) {
      return i->second;
    }
    return std::nullopt;
  }

  Access alloc_local(bool escape)
  {
    locals++;
    if(escape) {
      auto off = offset;
      assert(offset - word_size < offset); // overflow
      offset -= word_size;
      return InFrame(off);
    }
    else {
      return InReg(ir::TempGen::new_temp());
    }
  }

  uint16_t locals_count() const
  {
    return locals;
  }

  static std::string to_string(const Access& ax)
  {
    if(std::holds_alternative<InReg>(ax)) {
      return std::format("InReg(t{})", std::get<InReg>(ax).t);
    }
    else {
      return std::format("InFrame({})", std::get<InFrame>(ax).offset);
    }
  }

  static ir::Ex exp(const Access& fax, ir::Ex&& fp)
  {
    // translate an access into an exp
    if(std::holds_alternative<InFrame>(fax)) {
      auto at = std::make_unique<ir::tree::BinOpExp>(
        ir::tree::BinaryOp::plus,
        std::move(fp),
        std::make_unique<ir::tree::ConstExp>(std::get<InFrame>(fax).offset));
      return std::make_unique<ir::tree::MemExp>(std::move(at));
    }
    else {
      return std::make_unique<ir::tree::TempExp>(std::get<InReg>(fax).t);
    }
    assert(false);
  }

  static ir::Ex external_call(ir::TempGen::Label label,
                              std::vector<ir::Ex>&& args)
  {
    // TODO: external calls on Linux will use the system V abi
    // runtime functions are called using system V abi, unless
    // function attributes (cdecl) are specified

    return std::make_unique<ir::tree::CallExp>(
      std::make_unique<ir::tree::NameExp>(label), std::move(args));
  }

  private:
  int16_t offset{};
  std::vector<Access> formals_;
  ir::TempGen::Label label;
  uint16_t locals{};
};

} // namespace arch