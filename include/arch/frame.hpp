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
  static inline constexpr uint8_t word_size = 4;

  // TODO: map this to rbp?
  static inline auto FP = ir::TempGen::new_temp();
  // TODO: map this to rax?
  static inline auto RV = ir::TempGen::new_temp();

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
    // no need to do anything
    return std::make_unique<ir::tree::CallExp>(std::make_unique<ir::tree::NameExp>(label),
                                         std::move(args));
  }

  private:
  int16_t offset{};
  std::vector<Access> formals_;
  ir::TempGen::Label label;
  uint16_t locals{};
};

} // namespace arch