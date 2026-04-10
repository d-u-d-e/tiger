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
  static inline auto RV = TempGen::new_temp();

  Frame(const TempGen::Label&, const std::vector<bool>& f)
    : EP(TempGen::new_temp())
  {
    formals_ = std::vector(f.size(), 0);
  }

  static auto exp(const Access&, ir::Ex&&) -> ir::Ex
  {
    return {};
  }

  static auto external_call(const TempGen::Label&, std::vector<ir::Ex>&&) -> ir::Ex
  {
    return {};
  }

  static auto assembler_directives_begin() -> std::string
  {
    return "";
  }

  static auto assembler_directives_end() -> std::string
  {
    return "";
  }

  static auto emit_string(const ir::StringFragment&) -> std::string
  {
    return "";
  }

  static auto get_temporary_register_mapping()
    -> std::unordered_map<TempGen::Temp, assem::register_t>
  {
    return {};
  }

  [[nodiscard]] auto formals() const -> std::vector<Access>
  {
    return formals_;
  }

  static auto proc_entry_exit1(ir::tree::Stmt&&) -> ir::tree::Stmt
  {
    return {};
  }

  void proc_entry_exit2(std::list<assem::Instruction>&) { }

  static auto proc_entry_exit3(std::list<assem::Instruction>&)
    -> std::pair<std::string, std::string>
  {
    return {};
  }

  void rewrite_program(std::list<assem::Instruction>&, std::unordered_set<TempGen::Temp>) { }

  static auto name() -> TempGen::Label
  {
    return TempGen::new_label();
  }

  static auto alloc_local(bool) -> Access
  {
    return {};
  }

  [[nodiscard]] auto escaping_pointer() const -> TempGen::Temp
  {
    return EP;
  }

  private:
  std::vector<Access> formals_;
  TempGen::Temp EP;
};

} // namespace mock