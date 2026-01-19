#pragma once

#include <codegen/assem.hpp>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <list>
#include <memory>
#include <unordered_set>
#include <vector>

class Frame
{
  public:
  struct Access
  {
    virtual ~Access() = default;
  };

  using register_t = std::string;
  virtual ~Frame() = default;
  virtual std::vector<std::reference_wrapper<const Access>> formals() const = 0;
  virtual ir::TempGen::Temp frame_pointer() const = 0;
  virtual std::unordered_map<ir::TempGen::Temp, register_t> get_register_mapping() const = 0;
  virtual size_t number_of_registers() const = 0;
  virtual void rewrite_program(std::list<codegen::assem::Instruction>& list,
                               const std::unordered_set<ir::TempGen::Temp>& spilled_temps) = 0;
  virtual ir::TempGen::Label name() const = 0;
  virtual Access& alloc_local(bool escape) = 0;
  virtual size_t locals_count() const = 0;
  virtual ir::Ex exp(const Access& fax, ir::Ex&& fp) const = 0;
};

struct FrameFactory
{
  virtual ~FrameFactory() = default;
  virtual std::unique_ptr<Frame> make_frame(ir::TempGen::Label label,
                                            const std::vector<bool>& formals) const = 0;
};

struct Level
{
  struct Access
  {
    const Level* l{};
    const Frame::Access& fax;
  };

  Level(const Level* parent, std::unique_ptr<Frame> f)
    : parent(parent)
    , frame(std::move(f))
  {
    for(auto& formal : frame->formals())
    {
      formals.emplace_back(Access{this, formal.get()});
    }
  }

  std::vector<Access> formals;
  const Level* parent{};
  std::unique_ptr<Frame> frame;
};