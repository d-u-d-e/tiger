#pragma once
#include <memory>
#include <translation/temp.hpp>
#include <vector>

namespace arch
{

class Frame {
  public:
  using temp_t = translation::Temp::temp_t;
  using label_t = translation::Temp::label_t;

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

  Frame(label_t label, const std::vector<bool>& formals)
    : label(label)
  {
    for(auto escape : formals) {
      formals_.push_back(alloc_local(escape));
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
      return InReg(translation::Temp::getInstance().new_temp());
    }
  }

  private:
  uint16_t offset{0};
  std::vector<access_t> formals_;
  label_t label;
};

} // namespace arch