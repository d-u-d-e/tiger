#pragma once
#include <cassert>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <memory>
#include <vector>

namespace arch
{

class Frame {
  public:
  using temp_t = ir::Temp::temp_t;
  using label_t = ir::Temp::label_t;

  private:
  struct InReg {
    InReg(temp_t t)
      : t(t)
    { }
    temp_t t;
  };

  struct InFrame {
    // offset from the frame pointer
    InFrame(uint16_t offset)
      : offset(offset)
    { }
    uint16_t offset;
  };

  public:
  using access_t = std::variant<std::monostate, InReg, InFrame>;
  static inline constexpr uint8_t word_size = 4;

  // TODO: map this to rbp?
  static inline auto FP = ir::Temp::new_temp();

  Frame(label_t label, const std::vector<bool>& formals)
    : label(label)
  {
    for(auto escape : formals) {
      uint16_t off = 0;
      if(escape) {
        formals_.push_back(InFrame(off));
        off += word_size; // incoming params
      }
      else {
        formals_.push_back(InReg(ir::Temp::new_temp()));
      }
    }
  }

  const std::vector<access_t>& formals() const
  {
    return formals_;
  }

  label_t name() const
  {
    return label;
  }

  access_t alloc_local(bool escape)
  {
    if(escape) {
      auto off = offset;
      assert(offset - word_size < offset); // overflow
      offset -= word_size;
      return InFrame(off);
    }
    else {
      return InReg(ir::Temp::new_temp());
    }
  }

  static std::unique_ptr<ir::Exp> exp(access_t fax, std::unique_ptr<ir::Exp> fp)
  {
    // translate an access into an exp
    if(std::holds_alternative<InFrame>(fax)) {
      auto at = std::make_unique<ir::BinOpExp>(
        ir::BinaryOp::plus,
        std::move(fp),
        std::make_unique<ir::ConstExp>(std::get<InFrame>(fax).offset));
      return std::make_unique<ir::MemExp>(std::move(at));
    }
    else {
      return std::make_unique<ir::TempExp>(std::get<InReg>(fax).t);
    }
    assert(false);
  }

  private:
  uint16_t offset{0};
  std::vector<access_t> formals_;
  label_t label;
};

} // namespace arch