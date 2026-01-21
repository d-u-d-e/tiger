#pragma once
#include "ir/tree.hpp"
#include "temp.hpp"

namespace mock
{

class Frame
{
  public:
  using Access = int;
  static inline size_t word_size = 0;
  static inline auto FP = TempGen::new_temp();
  static inline auto RV = TempGen::new_temp();

  static ir::Ex exp(const Access&, ir::Ex&&)
  {
    return {};
  }
  Frame(TempGen::Label, const std::vector<bool>& f)

  {
    formals_ = std::vector(f.size(), 0);
  }

  std::vector<Access> formals() const
  {
    return formals_;
  }

  ir::tree::Stmt proc_entry_exit1(ir::tree::Stmt&&)
  {
    return {};
  }

  static ir::Ex external_call(TempGen::Label, std::vector<ir::Ex>&&)
  {
    return {};
  }

  TempGen::Label name() const
  {
    return TempGen::new_label();
  }

  Access alloc_local(bool)
  {
    return {};
  }

  private:
  std::vector<Access> formals_;
};

} // namespace mock