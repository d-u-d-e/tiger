#pragma once
#include "fragment.hpp"
#include "frame.hpp"
#include "ir/fragment.hpp"
#include "ir/pretty_printer.hpp"
#include "ir/tree.hpp"
#include "level.hpp"
#include "parser/ast.hpp"
#include "temp.hpp"
#include <memory>

namespace ir
{
template <IsFrame FrameT>
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
  Exp var(const LevelT::Access& ax, const LevelT* current);
  Exp seq_exp(std::vector<Exp>&& exps);
  Ex constant(int64_t constant);
  Exp call_exp(Exp&& closure, std::vector<Exp>&& args);
  Exp assign(Exp&& left, Exp&& right);
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
    const LevelT::Access& iax, Exp&& low, Exp&& high, Exp&& body, const TempGen::Label& lbreak);
  std::string dump_fragment(const FragmentT& f) const;
  LevelT::Access alloc_local(LevelT& level, bool escape);
  std::unique_ptr<LevelT>
  new_level(const LevelT* parent, TempGen::Label label, const std::vector<bool>& formals);
  auto formals(const LevelT& level)
  {
    // return a view of accesses without the static link, to be used by the analyzer
    // which is not aware of it
    return std::ranges::subrange(level.formals.begin() + 1, level.formals.end());
  }
  std::vector<FragmentT>&& fragments()
  {
    return std::move(fragments_);
  }

  private:
  tree::BinaryOp map_binary_operator(parser::ast::Operator op);
  tree::RelOp map_rel_operator(parser::ast::Operator op);

  private:
  std::shared_ptr<LevelT> lvl_outermost;
  std::shared_ptr<LevelT> lvl_main;
  std::vector<FragmentT> fragments_;
};

// implementations
template <IsFrame FrameT>
Translator<FrameT>::Translator()
{
  lvl_outermost = std::make_shared<LevelT>(
    nullptr,
    std::make_unique<FrameT>(TempGen::named_label("tiger_outermost"), std::vector<bool>{}));

  lvl_main = std::make_shared<LevelT>(
    lvl_outermost.get(),
    std::make_unique<FrameT>(TempGen::named_label("tiger_main"), std::vector<bool>{}));
}

template <IsFrame FrameT>
std::shared_ptr<Level<FrameT>> Translator<FrameT>::main_level()
{
  return lvl_main;
}

template <IsFrame FrameT>
std::shared_ptr<Level<FrameT>> Translator<FrameT>::outermost_level()
{
  return lvl_outermost;
}

template <IsFrame FrameT>
void Translator<FrameT>::add_fragment(FragmentT&& f)
{
  fragments_.emplace_back(std::move(f));
}

template <IsFrame FrameT>
void Translator<FrameT>::proc_entry_exit(std::shared_ptr<LevelT> level,
                                         Exp&& body)
{
  // move the body result onto the RV register
  auto rv = std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(FrameT::RV),
                                             unex(std::move(body)));

  // perform the view shift and add code to save and restore callee-saved registers
  auto pee1 = level->frame->proc_entry_exit1(std::move(rv));
  add_fragment(ProcedureFragment{std::move(pee1), std::move(level)});

  // proc_entry_exit2 and proc_entry_exit3 are called later after code generation
}

template <IsFrame FrameT>
Exp Translator<FrameT>::var(const LevelT::Access& var_ax, const LevelT* current)
{
  // TODO: use EP instead of FP

  tree::Exp fp_exp = std::make_unique<tree::TempExp>(FrameT::FP);
  while(var_ax.l != current)
  {
    // first arg holds the static link
    auto slink = current->formals[0].fax;
    fp_exp = FrameT::exp(slink, std::move(fp_exp));
    current = current->parent;
    assert(current != nullptr);
  }
  return FrameT::exp(var_ax.fax, std::move(fp_exp));
}

template <IsFrame FrameT>
tree::BinaryOp Translator<FrameT>::map_binary_operator(parser::ast::Operator op)
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

template <IsFrame FrameT>
tree::RelOp Translator<FrameT>::map_rel_operator(parser::ast::Operator op)
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

template <IsFrame FrameT>
Exp Translator<FrameT>::seq_exp(std::vector<Exp>&& exps)
{
  auto size = exps.size();
  if(0 == size)
  {
    return unnx(std::make_unique<tree::ConstExp>(0));
  }
  if(1 == size)
  {
    return std::move(exps[0]);
  }

  auto stmt_seq = unnx(std::move(exps[0]));

  for(size_t i = 1; i < size - 1; i++)
  {
    stmt_seq = std::make_unique<tree::SeqStmt>(std::move(stmt_seq), unnx(std::move(exps[i])));
  }
  return std::make_unique<tree::ESeqExp>(std::move(stmt_seq), unex(std::move(exps[size - 1])));
}

template <IsFrame FrameT>
Ex Translator<FrameT>::constant(int64_t constant)
{
  return std::make_unique<tree::ConstExp>(constant);
}

template <IsFrame FrameT>
Ex Translator<FrameT>::string(const std::string& value)
{
  auto lab = TempGen::new_label();
  add_fragment(ir::StringFragment{lab, value});
  return std::make_unique<tree::NameExp>(lab);
}

template <IsFrame FrameT>
Exp Translator<FrameT>::binary_exp(parser::ast::Operator op, Exp&& left, Exp&& right)
{
  Exp result;
  switch(op)
  {
  case parser::ast::Operator::plus:
  case parser::ast::Operator::minus:
  case parser::ast::Operator::divide:
  case parser::ast::Operator::times: {
    auto binop = map_binary_operator(op);
    result = std::make_unique<tree::BinOpExp>(binop, unex(std::move(left)), unex(std::move(right)));
    break;
  }
  default:
    assert(false);
  }
  return result;
}

template <IsFrame FrameT>
Exp Translator<FrameT>::rel_exp(parser::ast::Operator op, Exp&& left, Exp&& right)
{
  Exp result;
  switch(op)
  {
  case parser::ast::Operator::equal:
  case parser::ast::Operator::not_equal:
  case parser::ast::Operator::less_equal:
  case parser::ast::Operator::greater_equal:
  case parser::ast::Operator::less:
  case parser::ast::Operator::greater: {
    auto relop = map_rel_operator(op);
    result = [lex = unex(std::move(left)), rex = unex(std::move(right)), relop](
               TempGen::Label tlabel, TempGen::Label flabel) mutable {
      return std::make_unique<tree::CJumpStmt>(
        relop, std::move(lex), std::move(rex), tlabel, flabel);
    };
    break;
  }
  default:
    assert(false);
  }
  return result;
}

template <IsFrame FrameT>
Exp Translator<FrameT>::strings_equal(Exp&& left, Exp&& right)
{
  std::vector<Ex> args_as_ex;
  args_as_ex.emplace_back(unex(std::move(left)));
  args_as_ex.emplace_back(unex(std::move(right)));
  return FrameT::external_call(TempGen::named_label("string_equal"), std::move(args_as_ex));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::strings_nequal(Exp&& left, Exp&& right)
{
  return rel_exp(
    parser::ast::Operator::equal, strings_equal(std::move(left), std::move(right)), constant(0));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::array_subscript(Exp&& var, Exp&& index)
{
  // we basically need to compute mem(var + index * word_size)
  return std::make_unique<tree::MemExp>(std::make_unique<tree::BinOpExp>(
    tree::BinaryOp::plus,
    unex(std::move(var)),
    std::make_unique<tree::BinOpExp>(
      tree::BinaryOp::mul, unex(std::move(index)), constant(FrameT::word_size))));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::array_exp(Exp&& size, Exp&& init)
{
  std::vector<Ex> args;
  args.push_back(unex(std::move(size)));
  args.push_back(unex(std::move(init)));
  return FrameT::external_call(TempGen::named_label("init_array"), std::move(args));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::record_field(Exp&& var, size_t index)
{
  return std::make_unique<tree::MemExp>(std::make_unique<tree::BinOpExp>(
    tree::BinaryOp::plus, unex(std::move(var)), constant(index * FrameT::word_size)));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::record_exp(std::vector<Exp>&& fields)
{
  auto temp = TempGen::new_temp();
  std::vector<Ex> args_alloc;
  args_alloc.push_back(constant(fields.size()));

  // alloc space
  auto do_alloc = std::make_unique<tree::MoveStmt>(
    std::make_unique<tree::TempExp>(temp),
    FrameT::external_call(TempGen::named_label("alloc_record"), std::move(args_alloc)));

  if(fields.size() != 0)
  {
    // initialize all fields
    auto f0 = std::make_unique<tree::MoveStmt>(
      std::make_unique<tree::MemExp>(std::make_unique<tree::BinOpExp>(
        tree::BinaryOp::plus, std::make_unique<tree::TempExp>(temp), constant(0))),
      unex(std::move(fields[0])));
    auto seq = std::make_unique<tree::SeqStmt>(std::move(do_alloc), std::move(f0));

    for(size_t i = 1; i < fields.size(); i++)
    {
      auto fi = std::make_unique<tree::MoveStmt>(
        std::make_unique<tree::MemExp>(
          std::make_unique<tree::BinOpExp>(tree::BinaryOp::plus,
                                           std::make_unique<tree::TempExp>(temp),
                                           constant(i * FrameT::word_size))),
        unex(std::move(fields[i])));
      seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::move(fi));
    }
    return std::make_unique<tree::ESeqExp>(std::move(seq), std::make_unique<tree::TempExp>(temp));
  }
  return std::make_unique<tree::ESeqExp>(std::move(do_alloc),
                                         std::make_unique<tree::TempExp>(temp));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::if_then_else_exp(Exp&& cond, Exp&& texp, Exp&& fexp)
{
  if(std::holds_alternative<Ex>(texp) || std::holds_alternative<Ex>(fexp))
  {
    // at least one branch is an expression
    auto j = TempGen::new_label();
    auto tl = TempGen::new_label();
    auto fl = TempGen::new_label();
    auto temp = TempGen::new_temp();

    // cjump(cond, t, f); t:
    auto seq = std::make_unique<tree::SeqStmt>(uncx(std::move(cond))(tl, fl),
                                               std::make_unique<tree::LabelStmt>(tl));
    // mov(temp, texp)
    seq = std::make_unique<tree::SeqStmt>(
      std::move(seq),
      std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(temp),
                                       unex(std::move(texp))));
    // jump(j)
    seq = std::make_unique<tree::SeqStmt>(
      std::move(seq),
      std::make_unique<tree::JumpStmt>(std::make_unique<tree::NameExp>(j), std::vector{j}));

    // f:
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(fl));

    // mov(temp, fexp)
    seq = std::make_unique<tree::SeqStmt>(
      std::move(seq),
      std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(temp),
                                       unex(std::move(fexp))));

    // j:
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(j));

    // temp
    return std::make_unique<tree::ESeqExp>(std::move(seq), std::make_unique<tree::TempExp>(temp));
  }

  else if(std::holds_alternative<Cx>(texp) && std::holds_alternative<Cx>(fexp))
  {
    // both branches are conditionals
    return [cond = uncx(std::move(cond)), cthen = std::move(texp), celse = std::move(fexp)](
             TempGen::Label t, TempGen::Label f) mutable {
      auto t_ = TempGen::new_label();
      auto f_ = TempGen::new_label();

      // cjump(t_, f_); t_:
      auto seq =
        std::make_unique<tree::SeqStmt>(cond(t_, f_), std::make_unique<tree::LabelStmt>(t_));

      // cjump(cthen, t, f)
      seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::get<Cx>(cthen)(t, f));

      // f_:
      seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(f_));

      // cjump(celse, t, f):
      seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::get<Cx>(celse)(t, f));
      return seq;
    };
  }
  else
  {
    // both branches are statements
    assert(std::holds_alternative<Nx>(texp));
    assert(std::holds_alternative<Nx>(fexp));

    auto j = TempGen::new_label();
    auto t = TempGen::new_label();
    auto f = TempGen::new_label();
    // cjump(cond, t, f); t:
    auto seq = std::make_unique<tree::SeqStmt>(uncx(std::move(cond))(t, f),
                                               std::make_unique<tree::LabelStmt>(t));
    // (texp)
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), unnx(std::move(texp)));
    // jump(j)
    seq = std::make_unique<tree::SeqStmt>(
      std::move(seq),
      std::make_unique<tree::JumpStmt>(std::make_unique<tree::NameExp>(j), std::vector{j}));
    // f:
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(f));
    // (fexp)
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), unnx(std::move(fexp)));
    // j:
    return std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(j));
  }
}

template <IsFrame FrameT>
Exp Translator<FrameT>::if_then_exp(Exp&& cond, Exp&& texp)
{
  auto t = TempGen::new_label();
  auto f = TempGen::new_label();

  // cjump(cond, t, f); t:
  auto seq = std::make_unique<tree::SeqStmt>(uncx(std::move(cond))(t, f),
                                             std::make_unique<tree::LabelStmt>(t));
  // (ethen)
  seq = std::make_unique<tree::SeqStmt>(std::move(seq), unnx(std::move(texp)));

  // f:
  return std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(f));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::while_exp(Exp&& cond, Exp&& body, const TempGen::Label& lbreak)
{
  auto ltest = TempGen::new_label();
  auto t = TempGen::new_label();

  // ltest:; cjump(cond, t, lbreak)
  auto seq = std::make_unique<tree::SeqStmt>(std::make_unique<tree::LabelStmt>(ltest),
                                             uncx(std::move(cond))(t, lbreak));
  // t:
  seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(t));
  // (body)
  seq = std::make_unique<tree::SeqStmt>(std::move(seq), unnx(std::move(body)));
  // jump(ltest)
  seq = std::make_unique<tree::SeqStmt>(
    std::move(seq),
    std::make_unique<tree::JumpStmt>(std::make_unique<tree::NameExp>(ltest), std::vector{ltest}));
  // lbreak:
  return std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(lbreak));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::break_exp(const TempGen::Label& lbreak)
{
  return std::make_unique<tree::JumpStmt>(std::make_unique<tree::NameExp>(lbreak),
                                          std::vector{lbreak});
}

template <IsFrame FrameT>
Exp Translator<FrameT>::for_exp(
  const LevelT::Access& iax, Exp&& low, Exp&& high, Exp&& body, const TempGen::Label& lbreak)
{
  auto ltest = TempGen::new_label();
  auto t = TempGen::new_label();

  // id := low; ltest:
  auto ass = assign(var(iax, iax.l), std::move(low));
  auto seq =
    std::make_unique<tree::SeqStmt>(unnx(std::move(ass)), std::make_unique<tree::LabelStmt>(ltest));
  // cjump(id <= high, t, lbreak)
  seq = std::make_unique<tree::SeqStmt>(
    std::move(seq),
    std::make_unique<tree::CJumpStmt>(
      tree::RelOp::le, unex(var(iax, iax.l)), unex(std::move(high)), t, lbreak));

  // t:
  seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(t));
  // (body)
  seq = std::make_unique<tree::SeqStmt>(std::move(seq), unnx(std::move(body)));

  // id := id + 1
  auto inc =
    std::make_unique<tree::BinOpExp>(tree::BinaryOp::plus, unex(var(iax, iax.l)), constant((1)));

  seq =
    std::make_unique<tree::SeqStmt>(std::move(seq), unnx(assign(var(iax, iax.l), std::move(inc))));

  // jump(ltest)
  seq = std::make_unique<tree::SeqStmt>(
    std::move(seq),
    std::make_unique<tree::JumpStmt>(std::make_unique<tree::NameExp>(ltest), std::vector{ltest}));
  // lbreak:
  return std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(lbreak));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::assign(Exp&& left, Exp&& right)
{
  return std::make_unique<tree::MoveStmt>(unex(std::move(left)), unex(std::move(right)));
}

template <IsFrame FrameT>
Exp Translator<FrameT>::call_exp(Exp&& closure, std::vector<Exp>&& args)
{

  // A closure is implemented as a record like:
  // {
  //     machine label: int,
  //     EP: int
  // }

  // So we need to get the first field of the record

  std::vector<Ex> args_as_exp;
  auto label = std::make_unique<tree::MemExp>(unex(std::move(closure)));

  // TODO: pointer to environment? This is the first field of the escaping pointer

  for(auto& arg : args)
  {
    // unex all arguments
    args_as_exp.emplace_back(unex(std::move(arg)));
  }
  return std::make_unique<tree::CallExp>(std::move(label), std::move(args_as_exp));

  /*

  call_exp(TempGen::Label name,
                                  const LevelT* lcaller,
                                  const LevelT* lcallee,
                                  std::vector<Exp>&& args):


  std::vector<Ex> args_as_exp;
  bool is_external = lcallee == lvl_outermost.get();

  if(!is_external)
  {
    // not a library function (runtime)
    // pass the static link as first argument
    Ex fp = std::make_unique<tree::TempExp>(FrameT::FP);
    while(lcaller != lvl_main.get() && lcallee->parent != lcaller)
    {
      // first arg holds the static link
      auto slink = lcaller->formals[0].fax;
      fp = FrameT::exp(slink, std::move(fp));
      lcaller = lcaller->parent;
      assert(lcaller != nullptr);
    }
    args_as_exp.emplace_back(std::move(fp));
  }

  for(auto& arg : args)
  {
    // unex all arguments
    args_as_exp.emplace_back(unex(std::move(arg)));
  }

  if(is_external)
  {
    return FrameT::external_call(name, std::move(args_as_exp));
  }

  return std::make_unique<tree::CallExp>(std::make_unique<tree::NameExp>(name),
                                         std::move(args_as_exp));*/
}

template <class... Ts>
struct overloads : Ts...
{
  using Ts::operator()...;
};

template <IsFrame FrameT>
std::string Translator<FrameT>::dump_fragment(const FragmentT& f) const
{

  auto dump_proc_frag = [](const ir::ProcedureFragment<FrameT>& pf) -> std::string {
    tree::PrettyPrinter printer;
    auto result = std::format("frag function: {}, args: {}\n",
                              pf.level->frame->name().str(),
                              pf.level->frame->formals().size());
    auto ir_str = std::visit(printer, pf.body) + "\n";
    return ir_str;
  };

  auto dump_string_frag = [](const StringFragment& sf) -> std::string {
    auto result = std::format("frag string: {}, value: \"{}\"\n", sf.label.str(), sf.lit);
    return result;
  };

  return std::visit(overloads{dump_proc_frag, dump_string_frag}, f);
}

template <IsFrame FrameT>
Level<FrameT>::Access Translator<FrameT>::alloc_local(LevelT& level, bool escape)
{
  typename LevelT::Access ax{.l = &level, .fax = level.frame->alloc_local(escape)};
  return ax;
}

template <IsFrame FrameT>
std::unique_ptr<Level<FrameT>> Translator<FrameT>::new_level(const LevelT* parent,
                                                             TempGen::Label label,
                                                             const std::vector<bool>& formals)
{
  // augment the formals with the static link as first parameter
  std::vector<bool> with_slink(formals.size() + 1);
  std::copy(formals.begin(), formals.end(), with_slink.begin() + 1);
  with_slink[0] = true; // always escapes
  return std::make_unique<LevelT>(parent, std::make_unique<FrameT>(label, with_slink));
}

} // namespace ir