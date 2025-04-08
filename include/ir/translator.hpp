#pragma once
#include <algorithm>
#include <cassert>
#include <codegen/arch/frame.hpp>
#include <cstddef>
#include <cstdint>
#include <ir/fragment.hpp>
#include <ir/level.hpp>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <memory>
#include <parser/ast.hpp>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace ir
{

class Translator {
  public:
  Translator()
  {
    lvl_outermost =
      std::make_shared<Level>(nullptr, arch::Frame(TempGen::named_label("tiger_outermost"), {}));
    // TODO: formal arguments of main?
    lvl_main = new_level(lvl_outermost.get(), TempGen::named_label("tiger_main"), {});
  }

  std::shared_ptr<Level> main_level()
  {
    return lvl_main;
  }

  std::shared_ptr<Level> outermost_level()
  {
    return lvl_outermost;
  }

  static std::unique_ptr<Level>
  new_level(const Level* parent, TempGen::Label label, const std::vector<bool>& formals)
  {
    // augment the formals with the static link as first parameter
    std::vector<bool> with_slink(formals.size() + 1);
    std::copy(formals.begin(), formals.end(), with_slink.begin() + 1);
    with_slink[0] = true;
    return std::make_unique<Level>(parent, arch::Frame(label, with_slink));
  }

  static auto formals(const Level& level)
  {
    // return a view of accesses without the static link, to be used by the analyzer
    // which is not aware of it
    return std::ranges::subrange(level.formals.begin() + 1, level.formals.end());
  }

  static Level::Access alloc_local(Level& level, bool escape)
  {
    return Level::Access{.l = &level, .fax = level.f.alloc_local(escape)};
  }

  static Exp simple_var(const Level::Access& ax, const Level* current);
  Exp seq_exp(std::vector<Exp>&& exps);
  Ex constant(int64_t constant);
  Exp call_exp(TempGen::Label name,
               const Level* lcaller,
               const Level* lcallee,
               std::vector<Exp>&& args);
  Exp assign(Exp&& left, Exp&& right);
  void proc_entry_exit(const Level& level, Exp&& body);
  Exp binary_exp(parser::ast::Operator op, Exp&& left, Exp&& right);
  Exp rel_exp(parser::ast::Operator op, Exp&& left, Exp&& right);

  Ex string(const std::string& value);
  Exp strings_equal(Exp&& left, Exp&& right);
  Exp strings_nequal(Exp&& left, Exp&& right);

  Exp array_subscript(Exp&& var, Exp&& index);
  Exp array_exp(Exp&& size, Exp&& init);
  Exp record_field(Exp&& var, size_t index);
  Exp record_exp(std::vector<Exp>&& fields);
  Exp if_then_exp(Exp&& cond, Exp&& texp);
  Exp if_then_else_exp(Exp&& cond, Exp&& texp, Exp&& fexp);

  Exp while_exp(Exp&& cond, Exp&& body, const TempGen::Label& lbreak);
  Exp break_exp(const TempGen::Label& lbreak);
  Exp for_exp(
    const Level::Access& iax, Exp&& low, Exp&& high, Exp&& body, const TempGen::Label& lbreak);

  void add_fragment(Fragment&& f)
  {
    fragments_.emplace_back(std::move(f));
  }

  std::vector<Fragment>& fragments()
  {
    return fragments_;
  }

  std::string dump_fragment(const Fragment& f) const;
  Ex unex(Exp&& exp);

  private:
  Nx unnx(Exp&& exp);
  Cx uncx(Exp&& exp);

  tree::BinaryOp map_binary_operator(parser::ast::Operator op)
  {
    switch(op)
    {
    case parser::ast::Operator::plus:
      return tree::BinaryOp::plus;
    case parser::ast::Operator::minus:
      return tree::BinaryOp::minus;
    case parser::ast::Operator::times:
      return tree::BinaryOp::mul;
    case parser::ast::Operator::divide:
      return tree::BinaryOp::div;
    default:
      assert(false);
    }
  }

  tree::RelOp map_rel_operator(parser::ast::Operator op)
  {
    switch(op)
    {
    case parser::ast::Operator::equal:
      return tree::RelOp::eq;
    case parser::ast::Operator::not_equal:
      return tree::RelOp::ne;
    case parser::ast::Operator::greater_equal:
      return tree::RelOp::ge;
    case parser::ast::Operator::less_equal:
      return tree::RelOp::le;
    case parser::ast::Operator::greater:
      return tree::RelOp::gt;
    case parser::ast::Operator::less:
      return tree::RelOp::lt;
    default:
      assert(false);
    }
  }

  std::vector<Fragment> fragments_;
  std::shared_ptr<Level> lvl_outermost;
  std::shared_ptr<Level> lvl_main;
  std::unique_ptr<TempGen::Label> lbreak{};
};

} // namespace ir