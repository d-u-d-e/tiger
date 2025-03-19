#pragma once
#include <arch/frame.hpp>
#include <ir/fragment.hpp>
#include <ir/level.hpp>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <memory>

namespace ir
{

class Translator {
  public:
  Translator()
  {
    lvl_outermost = std::make_shared<Level>(
      nullptr, arch::Frame(Temp::named_label("outermost"), {}));
  }

  std::shared_ptr<Level> outermost_level()
  {
    return lvl_outermost;
  }

  static std::unique_ptr<Level> new_level(const Level* parent,
                                          Temp::label_t label,
                                          const std::vector<bool>& formals)
  {
    // augment the formals with the static link as first parameter
    std::vector<bool> with_slink(formals.size() + 1);
    std::copy(formals.begin(), formals.end(), with_slink.begin() + 1);
    with_slink[0] = true;
    return std::make_unique<Level>(parent, arch::Frame(label, with_slink));
  }

  static const std::vector<Level::Access>& formals(const Level& level)
  {
    return level.formals;
  }

  static Level::Access alloc_local(Level& level, bool escape)
  {
    return Level::Access{.l = &level, .fax = level.f.alloc_local(escape)};
  }

  exp_t simple_var(const Level::Access& ax, const Level* current);
  exp_t seq_exp(std::vector<ir::exp_t>&& exps);
  ex_t constant(int constant);
  exp_t call_exp(Temp::label_t flab, std::vector<ir::ex_t>&& args);
  ex_t string(const std::string& value);

  void proc_entry_exit(const Level& level, exp_t body);

  ex_t unex(exp_t&& exp);
  nx_t unnx(exp_t&& exp);
  cx_t uncx(exp_t&& exp);

  void add_fragment(Fragment&& f)
  {
    fragments_.emplace_back(std::move(f));
  }

  const std::vector<Fragment>& fragments() const
  {
    return fragments_;
  }

  std::string dump_fragment(const Fragment& f) const;

  private:
  std::vector<Fragment> fragments_;
  std::shared_ptr<Level> lvl_outermost;
};

} // namespace ir