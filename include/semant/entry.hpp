#pragma once
#include "ir/level.hpp"
#include "semant/types.hpp"
#include <format>
#include <memory>
#include <variant>

namespace semant
{

using namespace types;

template <typename FrameT>
class SimpleVarEntry
{
  public:
  explicit SimpleVarEntry(SharedType type, Level<FrameT>::Access access)
    : type(std::move(type))
    , access(std::move(access))
  { }

  SharedType type;
  Level<FrameT>::Access access; // tells where the variable resides in memory
};

template <typename FrameT>
class ClosureEntry
{
  public:
  explicit ClosureEntry(TempGen::Label name,
                        std::shared_ptr<FunctionType> func_type,
                        std::shared_ptr<Level<FrameT>> level,
                        Level<FrameT>::Access access)
    : label(name)
    , fun_type(std::move(func_type))
    , level(std::move(level))
    , access(std::move(access))
  { }

  TempGen::Label label;
  std::shared_ptr<FunctionType> fun_type;
  std::shared_ptr<Level<FrameT>> level{};
  Level<FrameT>::Access access; // tells where the variable resides in memory
};

template <typename FrameT>
struct VEntry
{
  std::string to_string() const
  {
    std::string result;
    if(std::holds_alternative<SimpleVarEntry<FrameT>>(v))
    {
      SimpleVarEntry<FrameT> ventry = std::get<SimpleVarEntry<FrameT>>(v);
      result = std::format("VarEntry{{{}}}", ventry.type->to_string());
    }
    else if(std::holds_alternative<ClosureEntry<FrameT>>(v))
    {
      ClosureEntry<FrameT> fentry = std::get<ClosureEntry<FrameT>>(v);
      result = std::format("FuncEntry{{{}}}(", fentry.label.str());
      auto arg_size = fentry.fun_type->formals.size();
      for(size_t i = 0; i < arg_size; i++)
      {
        result += fentry.fun_type->formals[i]->to_string() + (i == (arg_size - 1) ? "" : ", ");
      }
      result += std::format(") -> {}", fentry.fun_type->ret->to_string());
    }
    return result;
  }
  std::variant<std::monostate, SimpleVarEntry<FrameT>, ClosureEntry<FrameT>> v;
};

struct TEntry
{
  std::string to_string() const
  {
    return t->to_string();
  }
  SharedType t;
};
} // namespace semant