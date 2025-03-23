#pragma once
#include <arch/frame.hpp>
#include <ir/fragment.hpp>
#include <ir/level.hpp>
#include <ir/temp.hpp>
#include <ir/tree.hpp>
#include <memory>
#include <parser/ast.hpp>

namespace ir
{

class Translator {
  public:
  Translator()
  {
    lvl_outermost = std::make_shared<Level>(
      nullptr, arch::Frame(Temp::named_label("tiger_outermost"), {}));
    // TODO: formal arguments of main?
    lvl_main =
      new_level(lvl_outermost.get(), Temp::named_label("tiger_main"), {});
  }

  std::shared_ptr<Level> main_level()
  {
    return lvl_main;
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
  ex_t constant(size_t constant);
  exp_t call_exp(Temp::label_t name,
                 const ir::Level* lcaller,
                 const ir::Level* lcallee,
                 std::vector<ir::exp_t>&& args);
  exp_t assign(exp_t&& left, exp_t&& right);
  void proc_entry_exit(const Level& level, exp_t&& body);
  exp_t binary_exp(parser::ast::Operator op, exp_t&& left, exp_t&& right);
  exp_t rel_exp(parser::ast::Operator op, exp_t&& left, exp_t&& right);

  ex_t string(const std::string& value);
  exp_t strings_equal(exp_t&& left, exp_t&& right);
  exp_t strings_nequal(exp_t&& left, exp_t&& right);

  exp_t array_subscript(exp_t&& var, exp_t&& index);
  exp_t array_exp(exp_t&& size, exp_t&& init);
  exp_t record_field(exp_t&& var, size_t index);
  exp_t record_exp(std::vector<exp_t>&& fields);
  exp_t if_then_exp(exp_t&& cond, exp_t&& texp);
  exp_t if_then_else_exp(exp_t&& cond, exp_t&& texp, exp_t&& fexp);

  exp_t while_exp(exp_t&& cond, exp_t&& body, const Temp::label_t& lbreak);
  exp_t break_exp(const Temp::label_t& lbreak);

  void add_fragment(Fragment&& f)
  {
    fragments_.emplace_back(std::move(f));
  }

  const std::vector<Fragment>& fragments() const
  {
    return fragments_;
  }

  std::string dump_fragment(const Fragment& f) const;
  ex_t unex(exp_t&& exp);

  private:
  nx_t unnx(exp_t&& exp);
  cx_t uncx(exp_t&& exp);

  ir::BinaryOp map_binary_operator(parser::ast::Operator op)
  {
    switch(op) {
    case parser::ast::Operator::plus:
      return ir::BinaryOp::plus;
    case parser::ast::Operator::minus:
      return ir::BinaryOp::minus;
    case parser::ast::Operator::times:
      return ir::BinaryOp::mul;
    case parser::ast::Operator::divide:
      return ir::BinaryOp::div;
    default:
      assert(false);
    }
  }

  ir::RelOp map_rel_operator(parser::ast::Operator op)
  {
    switch(op) {
    case parser::ast::Operator::equal:
      return ir::RelOp::eq;
    case parser::ast::Operator::not_equal:
      return ir::RelOp::ne;
    case parser::ast::Operator::greater_equal:
      return ir::RelOp::ge;
    case parser::ast::Operator::less_equal:
      return ir::RelOp::le;
    case parser::ast::Operator::greater:
      return ir::RelOp::gt;
    case parser::ast::Operator::less:
      return ir::RelOp::lt;
    default:
      assert(false);
    }
  }

  std::vector<Fragment> fragments_;
  std::shared_ptr<Level> lvl_outermost;
  std::shared_ptr<Level> lvl_main;
  std::unique_ptr<Temp::label_t> lbreak{};
};

} // namespace ir