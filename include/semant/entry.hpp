#pragma once
#include "ir/level.hpp"
#include "semant/types.hpp"

namespace semant
{

using namespace types;

template <typename FrameT>
class VarEntry
{
  public:
  explicit VarEntry(SharedType type, Level<FrameT>::Access access)
    : type(std::move(type))
    , access(std::move(access))
  { }

  SharedType type;
  Level<FrameT>::Access access; // tells where the variable resides in memory
};

template <typename FrameT>
class FuncEntry
{
  public:
  explicit FuncEntry(TempGen::Label name,
                     std::vector<SharedType> formals,
                     SharedType result,
                     std::shared_ptr<Level<FrameT>> level)
    : label(name)
    , formals(std::move(formals))
    , result(std::move(result))
    , level(std::move(level))
  { }

  TempGen::Label label;
  std::vector<SharedType> formals;
  SharedType result;
  std::shared_ptr<Level<FrameT>> level{};
};

template <typename FrameT>
struct VEntry
{
  std::string to_string() const;
  std::variant<std::monostate, VarEntry<FrameT>, FuncEntry<FrameT>> v;
};

struct TEntry
{
  std::string to_string() const;
  SharedType t;
};
} // namespace semant