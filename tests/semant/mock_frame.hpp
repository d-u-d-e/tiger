#pragma once
#include "assem.hpp"
#include "ir/fragment.hpp"
#include "ir/tree.hpp"
#include "temp.hpp"
#include <unordered_map>
#include <unordered_set>

namespace mock
{

class Frame
{
  public:
  using Access = int;
  static inline size_t word_size = 0;
  static inline auto FP = TempGen::new_temp();
  static inline auto RV = TempGen::new_temp();

  Frame(TempGen::Label, const std::vector<bool>& f)
    : EP(TempGen::new_temp())
  {
    formals_ = std::vector(f.size(), 0);
  }

  static ir::Ex exp(const Access&, ir::Ex&&)
  {
    return {};
  }

  static ir::Ex external_call(TempGen::Label, std::vector<ir::Ex>&&)
  {
    return {};
  }

  static std::string assembler_directives_begin()
  {
    return "";
  }

  static std::string assembler_directives_end()
  {
    return "";
  }

  static std::string emit_string(ir::StringFragment)
  {
    return "";
  }

  static std::unordered_map<TempGen::Temp, assem::register_t> get_temporary_register_mapping()
  {
    return {};
  }

  std::vector<Access> formals() const
  {
    return formals_;
  }

  ir::tree::Stmt proc_entry_exit1(ir::tree::Stmt&&)
  {
    return {};
  }

  void proc_entry_exit2(std::list<assem::Instruction>&) { }

  std::pair<std::string, std::string> proc_entry_exit3(std::list<assem::Instruction>&)
  {
    return {};
  }

  void rewrite_program(std::list<assem::Instruction>&, std::unordered_set<TempGen::Temp>) { }

  TempGen::Label name() const
  {
    return TempGen::new_label();
  }

  Access alloc_local(bool)
  {
    return {};
  }

  TempGen::Temp escaping_pointer() const
  {
    return EP;
  }

  private:
  std::vector<Access> formals_;
  const TempGen::Temp EP;
};

} // namespace mock