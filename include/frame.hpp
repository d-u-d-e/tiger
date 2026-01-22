#pragma once

#include "assem.hpp"
#include "ir/fragment.hpp"
#include "ir/tree.hpp"
#include "temp.hpp"
#include <concepts>
#include <list>
#include <type_traits>
#include <unordered_set>
#include <vector>

template <typename FrameT>
concept IsFrame = requires
{
  {
    FrameT::FP
    } -> std::convertible_to<TempGen::Temp>;

  {
    FrameT::RV
    } -> std::convertible_to<TempGen::Temp>;

  {
    FrameT::word_size
    } -> std::convertible_to<unsigned int>;

  typename FrameT::Access;

  requires std::is_constructible_v<FrameT, TempGen::Label, std::vector<bool>>;

  // const member functions
  requires requires(const FrameT f)
  {
    {
      f.formals()
      } -> std::same_as<std::vector<typename FrameT::Access>>;

    {
      f.name()
      } -> std::same_as<TempGen::Label>;

    {
      f.locals_count()
      } -> std::convertible_to<unsigned int>;
  };

  // static functions
  {
    FrameT::exp(std::declval<const typename FrameT::Access&>(), std::declval<ir::Ex>())
    } -> std::same_as<ir::Ex>;

  {
    FrameT::external_call(std::declval<TempGen::Label>(), std::declval<std::vector<ir::Ex>>())
    } -> std::same_as<ir::Ex>;

  {
    FrameT::assembler_directives_begin()
    } -> std::convertible_to<std::string>;

  {
    FrameT::assembler_directives_end()
    } -> std::convertible_to<std::string>;

  {
    FrameT::emit_string(std::declval<ir::StringFragment>())
    } -> std::convertible_to<std::string>;

  {
    FrameT::get_temporary_register_mapping()
    } -> std::convertible_to<std::unordered_map<TempGen::Temp, assem::register_t>>;

  // non-const member functions
  requires requires(FrameT f)
  {
    {
      f.proc_entry_exit1(std::declval<ir::tree::Stmt>())
      } -> std::same_as<ir::tree::Stmt>;

    {
      f.proc_entry_exit2(std::declval<std::list<assem::Instruction>&>())
      } -> std::same_as<void>;

    {
      f.proc_entry_exit3(std::declval<std::list<assem::Instruction>&>())
      } -> std::same_as<std::pair<std::string, std::string>>;

    {
      f.rewrite_program(std::declval<std::list<assem::Instruction>&>(),
                        std::declval<std::unordered_set<TempGen::Temp>>())
      } -> std::same_as<void>;

    {
      f.alloc_local(std::declval<bool>())
      } -> std::same_as<typename FrameT::Access>;
  };
};
