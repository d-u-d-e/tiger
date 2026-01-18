#pragma once

#include <ir/temp.hpp>
#include <memory>
#include <vector>

// TODO: How should we make the translator, frames and other stuff not dependant on the target with inheritance

namespace example1
{

class FrameAccess
{ };

class Frame
{
  public:
  virtual ~Frame() = default;
  virtual const std::vector<FrameAccess*>& formals() const = 0;
};

class FrameFactory
{
  public:
  virtual ~FrameFactory() = default;
  virtual std::unique_ptr<Frame> make_frame(ir::TempGen::Label label,
                                            const std::vector<bool>& formals) const = 0;
};

struct Level
{
  struct Access
  {
    const Level* l{};
    FrameAccess& fax;
  };

  Level(const Level* parent, std::unique_ptr<Frame> f)
    : parent(parent)
    , frame(std::move(f))
  {
    for(auto& formal : frame->formals())
    {
      formals.emplace_back(this, *formal);
    }
  }

  std::vector<Access> formals;
  const Level* parent{};
  std::unique_ptr<Frame> frame;
};

class Translator
{
  public:
  Translator(const FrameFactory& ff)
    : frame_factory(ff)
  {
    lvl_outermost = std::make_shared<Level>(
      nullptr,
      frame_factory.make_frame(ir::TempGen::named_label("tiger_outermost"), std::vector<bool>{}));

    lvl_main = std::make_shared<Level>(
      nullptr,
      frame_factory.make_frame(ir::TempGen::named_label("tiger_main"), std::vector<bool>{}));
  }

  private:
  std::shared_ptr<Level> lvl_outermost;
  std::shared_ptr<Level> lvl_main;
  const FrameFactory& frame_factory;
};

} // namespace example1

// TODO: How should we make the translator, frames and other stuff not dependant on the target with templates and traits
namespace example2
{

}