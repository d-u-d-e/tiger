#include "arch/x86_64/frame.hpp"
#include <algorithm>
#include <cassert>

namespace arch
{

X86Frame::X86Frame(TempGen::Label label, const std::vector<bool>& formals)
  : label(label)
{
  // just a dummy stmt
  view_shift = std::make_unique<ir::tree::ExpStmt>(std::make_unique<ir::tree::ConstExp>(0));

  //TODO
  static_cast<void>(formals);
}

std::vector<X86Frame::Access> X86Frame::formals() const
{
  return formals_;
}

ir::tree::Stmt X86Frame::proc_entry_exit1(ir::tree::Stmt&& stmt)
{
  // proc_entry_exit1 does the following:
  // - mov incoming register formal params to the place expected by the function (view shift)
  // - save callee saved registers
  // - restore callee saved registers

  std::vector<ir::tree::Stmt> save;
  std::vector<ir::tree::Stmt> restore;

  // since the reg allocator implements spilling, callee saved regs are not pushed to the stack
  // but moved to a temporary
  for(auto& reg : callee_saved)
  {
    auto t = TempGen::new_temp();
    save.push_back(std::make_unique<ir::tree::MoveStmt>(std::make_unique<ir::tree::TempExp>(t),
                                                        std::make_unique<ir::tree::TempExp>(reg)));
    restore.push_back(std::make_unique<ir::tree::MoveStmt>(std::make_unique<ir::tree::TempExp>(reg),
                                                           std::make_unique<ir::tree::TempExp>(t)));
  }

  auto folder = [](auto&& arg1, auto&& arg2) {
    return ir::tree::Stmt(std::make_unique<ir::tree::SeqStmt>(std::move(arg1), std::move(arg2)));
  };

  auto save_seq = std::ranges::fold_left_first(
    std::make_move_iterator(save.begin()), std::make_move_iterator(save.end()), folder);

  auto restore_seq = std::ranges::fold_left_first(
    std::make_move_iterator(restore.begin()), std::make_move_iterator(restore.end()), folder);

  assert(save_seq.has_value());
  assert(restore_seq.has_value());

  // view-shift -> save sequence -> body -> restore sequence
  stmt = std::make_unique<ir::tree::SeqStmt>(std::move(save_seq.value()), std::move(stmt));
  stmt = std::make_unique<ir::tree::SeqStmt>(std::move(stmt), std::move(restore_seq.value()));
  stmt = std::make_unique<ir::tree::SeqStmt>(std::move(view_shift), std::move(stmt));
  return stmt;
}

} // namespace arch