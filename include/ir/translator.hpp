#pragma once
#include "ir/fragment.hpp"
#include "ir/tree.hpp"
#include "level.hpp"
#include "temp.hpp"
#include <memory>

namespace ir
{
template <typename FrameT>
class Translator
{
  public:
  using LevelT = Level<FrameT>;
  using FragmentT = Fragment<FrameT>;
  using Frame = FrameT;

  Translator();
  std::shared_ptr<LevelT> main_level();
  std::shared_ptr<LevelT> outermost_level();
  void add_fragment(FragmentT&& f);
  void proc_entry_exit(std::shared_ptr<LevelT> level, Exp&& body);
  void translate_main_program(Exp&& exp)
  {
    proc_entry_exit(lvl_main, std::move(exp));
  }

  private:
  std::shared_ptr<LevelT> lvl_outermost;
  std::shared_ptr<LevelT> lvl_main;
  std::vector<FragmentT> fragments_;
};

// implementations
template <typename FrameT>
Translator<FrameT>::Translator()
{
  lvl_outermost = std::make_shared<LevelT>(
    nullptr,
    std::make_unique<FrameT>(TempGen::named_label("tiger_outermost"), std::vector<bool>{}));

  lvl_main = std::make_shared<LevelT>(
    lvl_outermost.get(),
    std::make_unique<FrameT>(TempGen::named_label("tiger_main"), std::vector<bool>{}));
}

template <typename FrameT>
std::shared_ptr<Level<FrameT>> Translator<FrameT>::main_level()
{
  return lvl_main;
}

template <typename FrameT>
std::shared_ptr<Level<FrameT>> Translator<FrameT>::outermost_level()
{
  return lvl_outermost;
}

template <typename FrameT>
void Translator<FrameT>::add_fragment(FragmentT&& f)
{
  fragments_.emplace_back(std::move(f));
}

template <typename FrameT>
void Translator<FrameT>::proc_entry_exit(std::shared_ptr<LevelT> level, Exp&& body)
{
  // move the body result onto the RV register
  auto rv = std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(FrameT::RV),
                                             unex(std::move(body)));

  // perform the view shift and add code to save and restore callee-saved registers
  auto pee1 = level->frame->proc_entry_exit1(std::move(rv));
  add_fragment(ProcedureFragment{std::move(pee1), std::move(level)});

  // proc_entry_exit2 and proc_entry_exit3 are called later after code generation
}

} // namespace ir