#pragma once
#include <codegen/arch.hpp>
#include <memory>
#include <unistd.h>
#include <utility>
#include <vector>

namespace ir
{

struct Level
{
  struct Access
  {
    const Level* l{};
    arch::Frame::Access fax;
  };

  Level(const Level* parent, std::unique_ptr<arch::Frame> f)
    : parent(parent)
    , frame(std::move(f))
  {
    for(auto& formal : frame->formals())
    {
      formals.emplace_back(this, formal);
    }
  }

  std::vector<Access> formals;
  const Level* parent{};
  std::unique_ptr<arch::Frame> frame;
};

template <typename Frame>
requires requires(const Frame f)
{
  typename Frame::Access;
  {
    f.formals()
    } -> std::same_as<std::vector<typename Frame::Access>>;
}
struct LevelT
{
  using Formals = std::vector<typename Frame::Access>;

  struct Access
  {
    const LevelT* l{};
    typename Frame::Access fax;
  };

  LevelT(const LevelT* parent, std::unique_ptr<Frame> f)
    : parent(parent)
    , frame(std::move(f))
  {
    for(Formals const& fs = frame->formals(); auto& formal : fs)
    {
      formals.emplace_back(this, formal);
    }
  }

  std::vector<Access> formals;
  const LevelT* parent{};
  std::unique_ptr<Frame> frame;
};

class FrameExample
{
  public:
  class Access
  { };
  std::vector<Access> formals() const
  {
    return {};
  }
};

template struct LevelT<FrameExample>;
using stype = int;

template <typename Access>
class VarEntryT
{
  public:
  explicit VarEntryT(stype type, Access access)
    : type(std::move(type))
    , access(std::move(access))
  { }

  stype type;
  Access access; // tells where the variable resides in memory
};

template <typename T>
class FuncEntryT
{
  public:
  explicit FuncEntryT(ir::TempGen::Label name,
                     std::vector<stype> formals,
                     stype result,
                     std::shared_ptr<T> level)
    : label(name)
    , formals(std::move(formals))
    , result(std::move(result))
    , level(std::move(level))
  { }

  ir::TempGen::Label label;
  std::vector<stype> formals;
  stype result;
  std::shared_ptr<T> level{};
};

template <typename L>
struct VEntryT
{
  std::string to_string() const;
  std::variant<std::monostate, VarEntryT<typename L::Access>, FuncEntryT<L>> v;
};

} // namespace ir