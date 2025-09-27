#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <ir/fragment.hpp>
#include <ir/level.hpp>
#include <ir/pretty_printer.hpp>
#include <ir/temp.hpp>
#include <ir/translator.hpp>
#include <ir/tree.hpp>
#include <memory>
#include <parser/ast.hpp>
#include <string>
#include <utility>
#include <variant>
#include <vector>

template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

namespace ir
{

Exp Translator::simple_var(const Level::Access& var_ax, const Level* current)
{
  tree::Exp fp_exp = std::make_unique<tree::TempExp>(arch::Frame::FP);

  while(var_ax.l != current)
  {
    // first arg holds the static link
    auto slink = current->formals[0].fax;
    fp_exp = arch::Frame::exp(slink, std::move(fp_exp));
    current = current->parent;
    assert(current != nullptr);
  }
  return arch::Frame::exp(var_ax.fax, std::move(fp_exp));
}

Exp Translator::seq_exp(std::vector<Exp>&& exps)
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

Ex Translator::constant(int64_t constant)
{
  return std::make_unique<tree::ConstExp>(constant);
}

Ex Translator::string(const std::string& value)
{
  auto lab = TempGen::new_label();
  add_fragment(StringFragment{lab, value});
  return std::make_unique<tree::NameExp>(lab);
}

Exp Translator::binary_exp(parser::ast::Operator op, Exp&& left, Exp&& right)
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

Exp Translator::rel_exp(parser::ast::Operator op, Exp&& left, Exp&& right)
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

Exp Translator::strings_equal(Exp&& left, Exp&& right)
{
  std::vector<Ex> args_as_ex;
  args_as_ex.emplace_back(unex(std::move(left)));
  args_as_ex.emplace_back(unex(std::move(right)));
  return arch::Frame::external_call(TempGen::named_label("string_equal"), std::move(args_as_ex));
}

Exp Translator::strings_nequal(Exp&& left, Exp&& right)
{
  return rel_exp(
    parser::ast::Operator::equal, strings_equal(std::move(left), std::move(right)), constant(0));
}

Exp Translator::array_subscript(Exp&& var, Exp&& index)
{
  // we basically need to compute mem(var + index * word_size)
  return std::make_unique<tree::MemExp>(std::make_unique<tree::BinOpExp>(
    tree::BinaryOp::plus,
    unex(std::move(var)),
    std::make_unique<tree::BinOpExp>(
      tree::BinaryOp::mul, unex(std::move(index)), constant(arch::Frame::word_size))));
}

Exp Translator::array_exp(Exp&& size, Exp&& init)
{
  std::vector<Ex> args;
  args.push_back(unex(std::move(size)));
  args.push_back(unex(std::move(init)));
  return arch::Frame::external_call(TempGen::named_label("init_array"), std::move(args));
}

Exp Translator::record_field(Exp&& var, size_t index)
{
  return std::make_unique<tree::MemExp>(std::make_unique<tree::BinOpExp>(
    tree::BinaryOp::plus, unex(std::move(var)), constant(index * arch::Frame::word_size)));
}

Exp Translator::record_exp(std::vector<Exp>&& fields)
{
  auto temp = TempGen::new_temp();
  std::vector<Ex> args_alloc;
  args_alloc.push_back(constant(fields.size()));

  // alloc space
  auto do_alloc = std::make_unique<tree::MoveStmt>(
    std::make_unique<tree::TempExp>(temp),
    arch::Frame::external_call(TempGen::named_label("alloc_record"), std::move(args_alloc)));

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
                                           constant(i * arch::Frame::word_size))),
        unex(std::move(fields[i])));
      seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::move(fi));
    }
    return std::make_unique<tree::ESeqExp>(std::move(seq), std::make_unique<tree::TempExp>(temp));
  }
  return std::make_unique<tree::ESeqExp>(std::move(do_alloc),
                                         std::make_unique<tree::TempExp>(temp));
}

Exp Translator::if_then_else_exp(Exp&& cond, Exp&& texp, Exp&& fexp)
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

Exp Translator::if_then_exp(Exp&& cond, Exp&& texp)
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

Exp Translator::while_exp(Exp&& cond, Exp&& body, const TempGen::Label& lbreak)
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

Exp Translator::break_exp(const TempGen::Label& lbreak)
{
  return std::make_unique<tree::JumpStmt>(std::make_unique<tree::NameExp>(lbreak),
                                          std::vector{lbreak});
}

Exp Translator::for_exp(
  const Level::Access& iax, Exp&& low, Exp&& high, Exp&& body, const TempGen::Label& lbreak)
{
  auto ltest = TempGen::new_label();
  auto t = TempGen::new_label();

  // id := low; ltest:
  auto ass = assign(simple_var(iax, iax.l), std::move(low));
  auto seq =
    std::make_unique<tree::SeqStmt>(unnx(std::move(ass)), std::make_unique<tree::LabelStmt>(ltest));
  // cjump(id <= high, t, lbreak)
  seq = std::make_unique<tree::SeqStmt>(
    std::move(seq),
    std::make_unique<tree::CJumpStmt>(
      tree::RelOp::le, unex(simple_var(iax, iax.l)), unex(std::move(high)), t, lbreak));

  // t:
  seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(t));
  // (body)
  seq = std::make_unique<tree::SeqStmt>(std::move(seq), unnx(std::move(body)));

  // id := id + 1
  auto inc = std::make_unique<tree::BinOpExp>(
    tree::BinaryOp::plus, unex(simple_var(iax, iax.l)), constant((1)));

  seq = std::make_unique<tree::SeqStmt>(std::move(seq),
                                        unnx(assign(simple_var(iax, iax.l), std::move(inc))));

  // jump(ltest)
  seq = std::make_unique<tree::SeqStmt>(
    std::move(seq),
    std::make_unique<tree::JumpStmt>(std::make_unique<tree::NameExp>(ltest), std::vector{ltest}));
  // lbreak:
  return std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(lbreak));
}

Exp Translator::assign(Exp&& left, Exp&& right)
{
  return std::make_unique<tree::MoveStmt>(unex(std::move(left)), unex(std::move(right)));
}

void Translator::proc_entry_exit(std::shared_ptr<Level> level, Exp&& body)
{
  // move the body result onto the RV register
  auto rv = std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(arch::Frame::RV),
                                             unex(std::move(body)));

  // perform the view shift and add code to save and restore callee-saved registers
  auto pee1 = level->frame->proc_entry_exit1(std::move(rv));
  add_fragment(ProcedureFragment{std::move(pee1), std::move(level)});

  // proc_entry_exit2 and proc_entry_exit3 are called later after code generation
}

Exp Translator::call_exp(TempGen::Label name,
                         const Level* lcaller,
                         const Level* lcallee,
                         std::vector<Exp>&& args)
{
  std::vector<Ex> args_as_exp;
  bool is_external = lcallee == lvl_outermost.get();

  if(!is_external)
  {
    // not a library function (runtime)
    // pass the static link as first argument
    Ex fp = std::make_unique<tree::TempExp>(arch::Frame::FP);
    while(lcaller != lvl_main.get() && lcallee->parent != lcaller)
    {
      // first arg holds the static link
      auto slink = lcaller->formals[0].fax;
      fp = arch::Frame::exp(slink, std::move(fp));
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
    return arch::Frame::external_call(name, std::move(args_as_exp));
  }

  return std::make_unique<tree::CallExp>(std::make_unique<tree::NameExp>(name),
                                         std::move(args_as_exp));
}

Ex Translator::unex(Exp&& exp)
{
  // unex(nx) is just ESeqExp(nx, 0)
  // unex(ex) is just ex
  // unex(cx) is:
  // ESeq(SeqStmt[MoveStmt(temp, 1), cx(t, f), LabelStmt(f), MoveStmt(temp, 0), LabelStmt(t)], temp)

  if(std::holds_alternative<Ex>(exp))
  {
    return std::move(std::get<Ex>(exp));
  }
  else if(std::holds_alternative<Nx>(exp))
  {
    return std::make_unique<tree::ESeqExp>(std::move(std::get<Nx>(exp)), constant(0));
  }
  else if(std::holds_alternative<Cx>(exp))
  {
    auto cx = std::move(std::get<Cx>(exp));
    auto temp = TempGen::new_temp();
    auto tlab = TempGen::new_label();
    auto flab = TempGen::new_label();
    auto seq = std::make_unique<tree::SeqStmt>(
      std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(temp),
                                       std::make_unique<tree::ConstExp>(1)),
      cx(tlab, flab));
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(flab));
    seq = std::make_unique<tree::SeqStmt>(
      std::move(seq),
      std::make_unique<tree::MoveStmt>(std::make_unique<tree::TempExp>(temp),
                                       std::make_unique<tree::ConstExp>(0)));
    seq = std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(tlab));
    return std::make_unique<tree::ESeqExp>(std::move(seq), std::make_unique<tree::TempExp>(temp));
  }
  assert(false);
  std::unreachable();
}

Nx Translator::unnx(Exp&& exp)
{
  // unnx(nx) is just nx
  // unnx(ex) is a ExpStmt(ex)
  // unnx(cx) is Seq[cx(t, f), LabelStmt(t), LabelStmt(f)]

  if(std::holds_alternative<Ex>(exp))
  {
    return std::make_unique<tree::ExpStmt>(std::move(std::get<Ex>(exp)));
  }
  else if(std::holds_alternative<Nx>(exp))
  {
    return std::move(std::get<Nx>(exp));
  }
  else if(std::holds_alternative<Cx>(exp))
  {
    auto tlab = TempGen::new_label();
    auto flab = TempGen::new_label();
    auto seq = std::make_unique<tree::SeqStmt>(std::get<Cx>(exp)(tlab, flab),
                                               std::make_unique<tree::LabelStmt>(tlab));
    return std::make_unique<tree::SeqStmt>(std::move(seq), std::make_unique<tree::LabelStmt>(flab));
  }
  assert(false);
  std::unreachable();
}

Cx Translator::uncx(Exp&& exp)
{
  // uncx(nx) should not occur in a valid program
  // uncx(ex) is: (t, f) -> CJumpStmt(eq, ex, 0, f, t)
  // uncx(cx) is just cx

  if(std::holds_alternative<Ex>(exp))
  {
    return [e = std::move(exp)](TempGen::Label t, TempGen::Label f) mutable {
      return std::make_unique<tree::CJumpStmt>(
        tree::RelOp::eq, std::move(std::get<Ex>(e)), std::make_unique<tree::ConstExp>(0), f, t);
    };
  }
  else if(std::holds_alternative<Cx>(exp))
  {
    return std::move(std::get<Cx>(exp));
  }

  assert(false);
  std::unreachable();
}

std::string Translator::dump_fragment(const Fragment& f) const
{
  auto dump_proc_frag = [](const ProcedureFragment& pf) -> std::string {
    tree::PrettyPrinter printer;
    auto result = std::format("frag function: {}, args: {}, locals: {}\n",
                              pf.level->frame->name().str(),
                              pf.level->frame->formals().size(),
                              pf.level->frame->locals_count());
    auto ir_str = std::visit(printer, pf.body) + "\n";
    return ir_str;
  };

  auto dump_string_frag = [](const StringFragment& sf) -> std::string {
    auto result = std::format("frag string: {}, value: \"{}\"\n", sf.label.str(), sf.lit);
    return result;
  };

  return std::visit(overloads{dump_proc_frag, dump_string_frag}, f);
}

} // namespace ir