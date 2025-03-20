#include <arch/frame.hpp>
#include <cassert>
#include <ir/pretty_printer.hpp>
#include <ir/translator.hpp>
#include <utility>

namespace ir
{

ir::exp_t Translator::simple_var(const Level::Access& var_ax,
                                 const Level* current)
{
  std::unique_ptr<Exp> fp = std::make_unique<TempExp>(arch::Frame::FP);

  while(var_ax.l != current) {
    // first arg holds the static link
    auto slink = current->formals[0].fax;
    fp = arch::Frame::exp(slink, std::move(fp));
    current = current->parent;
    assert(current != nullptr);
  }
  return arch::Frame::exp(var_ax.fax, std::move(fp));
}

ir::exp_t Translator::seq_exp(std::vector<ir::exp_t>&& exps)
{
  auto size = exps.size();
  if(0 == size) {
    return std::make_unique<ir::ConstExp>(0);
  }
  else if(1 == size) {
    return std::move(exps[0]);
  }

  auto stmt_seq = unnx(std::move(exps[0]));

  for(auto i = 1; i < size - 1; i++) {
    stmt_seq = std::make_unique<ir::SeqStmt>(std::move(stmt_seq),
                                             unnx(std::move(exps[i])));
  }

  return std::make_unique<ir::ESeqExp>(move(stmt_seq),
                                       unex(std::move(exps[size - 1])));
}

ex_t Translator::constant(int constant)
{
  return std::make_unique<ir::ConstExp>(constant);
}

ex_t Translator::string(const std::string& value)
{
  auto lab = Temp::new_label();
  add_fragment(StringFragment{lab, value});
  return std::make_unique<ir::NameExp>(lab);
}

exp_t Translator::binary_exp(parser::ast::Operator op,
                             exp_t&& left,
                             exp_t&& right)
{
  exp_t result;
  switch(op) {
  case parser::ast::Operator::plus:
  case parser::ast::Operator::minus:
  case parser::ast::Operator::divide:
  case parser::ast::Operator::times: {
    auto binop = map_binary_operator(op);
    result = std::make_unique<ir::BinOpExp>(
      binop, unex(std::move(left)), unex(std::move(right)));
    break;
  }
  default:
    assert(false);
  }
  return result;
}

exp_t Translator::rel_exp(parser::ast::Operator op, exp_t&& left, exp_t&& right)
{
  exp_t result;
  switch(op) {
  case parser::ast::Operator::equal:
  case parser::ast::Operator::not_equal:
  case parser::ast::Operator::less_equal:
  case parser::ast::Operator::greater_equal:
  case parser::ast::Operator::less:
  case parser::ast::Operator::greater: {
    auto relop = map_rel_operator(op);
    result = [lex = unex(std::move(left)), rex = unex(std::move(right)), relop](
               Temp::label_t tlabel, Temp::label_t flabel) mutable {
      return std::make_unique<ir::CJumpStmt>(
        relop, std::move(lex), std::move(rex), tlabel, flabel);
    };
    break;
  }
  default:
    assert(false);
  }
  return result;
}

exp_t Translator::strings_equal(exp_t&& left, exp_t&& right)
{
  std::vector<ex_t> args_as_ex;
  args_as_ex.emplace_back(unex(std::move(left)));
  args_as_ex.emplace_back(unex(std::move(right)));
  return arch::Frame::external_call(Temp::named_label("stringEqual"),
                                    std::move(args_as_ex));
}

exp_t Translator::strings_nequal(exp_t&& left, exp_t&& right)
{
  return rel_exp(parser::ast::Operator::equal,
                 strings_equal(std::move(left), std::move(right)),
                 constant(0));
}

exp_t Translator::assign(exp_t&& left, exp_t&& right)
{
  return std::make_unique<ir::MoveStmt>(unex(std::move(left)),
                                        unex(std::move(right)));
}

void Translator::proc_entry_exit(const Level& level, exp_t&& body)
{
  add_fragment(ProcedureFragment{unnx(std::move(body)), level.f});
}

exp_t Translator::call_exp(const ir::Level* caller,
                           const ir::Level* callee,
                           std::vector<ir::exp_t>&& args)
{
  // unex all arguments
  std::vector<ir::ex_t> args_as_exp(args.size() + 1);
  std::transform(args.begin(),
                 args.end(),
                 args_as_exp.begin() + 1,
                 [this](auto& a) { return unex(std::move(a)); });
  

  // TODO: handle calls to library functions, defined in outermost level
  
  // pass the static link as first argument
  // we need to compute it by going through the caller's link until we hit the level of the callee
  // it may happen that we are inside a recursive function, so caller level == callee level

  ex_t fp = std::make_unique<TempExp>(arch::Frame::FP);
  while(callee != caller && caller != lvl_main.get()) {
    // first arg holds the static link
    auto slink = caller->formals[0].fax;
    fp = arch::Frame::exp(slink, std::move(fp));
    caller = caller->parent;
    assert(caller != nullptr);
  }
  args_as_exp[0] = arch::Frame::exp(caller->formals[0].fax, std::move(fp));

  return std::make_unique<ir::CallExp>(
    std::make_unique<NameExp>(callee->f.name()), std::move(args_as_exp));
}

ex_t Translator::unex(exp_t&& exp)
{
  // unex(nx) is just ESeqExp(nx, 0)
  // unex(ex) is just ex
  // unex(cx) is:
  // ESeq(SeqStmt[MoveStmt(temp, 1), cx(t, f), LabelStmt(f), MoveStmt(temp, 0), LabelStmt(t)], temp)

  if(std::holds_alternative<ex_t>(exp)) {
    return std::move(std::get<ex_t>(exp));
  }
  else if(std::holds_alternative<nx_t>(exp)) {
    return std::make_unique<ir::ESeqExp>(std::move(std::get<nx_t>(exp)),
                                         constant(0));
  }
  else if(std::holds_alternative<cx_t>(exp)) {
    auto cx = std::move(std::get<cx_t>(exp));
    auto temp = ir::Temp::new_temp();
    auto tlab = ir::Temp::new_label();
    auto flab = ir::Temp::new_label();
    auto seq = std::make_unique<ir::SeqStmt>(
      std::make_unique<MoveStmt>(std::make_unique<TempExp>(temp),
                                 std::make_unique<ConstExp>(1)),
      cx(tlab, flab));
    seq = std::make_unique<ir::SeqStmt>(std::move(seq),
                                        std::make_unique<ir::LabelStmt>(flab));
    seq = std::make_unique<ir::SeqStmt>(
      std::move(seq),
      std::make_unique<MoveStmt>(std::make_unique<TempExp>(temp),
                                 std::make_unique<ConstExp>(0)));
    seq = std::make_unique<ir::SeqStmt>(std::move(seq),
                                        std::make_unique<ir::LabelStmt>(tlab));
    return std::make_unique<ir::ESeqExp>(std::move(seq),
                                         std::make_unique<TempExp>(temp));
  }
  // TODO this breaks the tests
  //assert(false);
  //std::unreachable();
  return nullptr;
}

nx_t Translator::unnx(exp_t&& exp)
{
  // unnx(nx) is just nx
  // unnx(ex) is a ExpStmt(ex)
  // unnx(cx) is Seq[cx(t, f), LabelStmt(t), LabelStmt(f)]

  if(std::holds_alternative<ex_t>(exp)) {
    return std::make_unique<ir::ExpStmt>(std::move(std::get<ex_t>(exp)));
  }
  else if(std::holds_alternative<nx_t>(exp)) {
    return std::move(std::get<nx_t>(exp));
  }
  else if(std::holds_alternative<cx_t>(exp)) {
    auto tlab = ir::Temp::new_label();
    auto flab = ir::Temp::new_label();
    auto seq = std::make_unique<ir::SeqStmt>(
      std::get<cx_t>(exp)(tlab, flab), std::make_unique<ir::LabelStmt>(tlab));
    return std::make_unique<ir::SeqStmt>(std::move(seq),
                                         std::make_unique<ir::LabelStmt>(flab));
  }
  // TODO this breaks the tests
  //assert(false);
  //std::unreachable();
  return nullptr;
}

cx_t Translator::uncx(exp_t&& exp)
{
  // uncx(nx) should not occur in a valid program
  // uncx(ex) is: (t, f) -> CJumpStmt(eq, ex, 0, f, t)
  // uncx(cx) is just cx

  if(std::holds_alternative<ex_t>(exp)) {
    return [&exp](Temp::label_t t, Temp::label_t f) -> nx_t {
      return std::make_unique<ir::CJumpStmt>(ir::RelOp::eq,
                                             std::move(std::get<ex_t>(exp)),
                                             std::make_unique<ir::ConstExp>(0),
                                             f,
                                             t);
    };
  }
  else if(std::holds_alternative<cx_t>(exp)) {
    return std::move(std::get<cx_t>(exp));
  }

  assert(false);
  std::unreachable();
}

template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

std::string Translator::dump_fragment(const Fragment& f) const
{
  auto dump_proc_frag = [](const ProcedureFragment& pf) -> std::string {
    ir::PrettyPrinter printer;
    auto result = std::format("frag function: {}, args: {}, locals: {}\n",
                              pf.frame.name().str(),
                              pf.frame.formals().size(),
                              pf.frame.locals_count());
    auto ir_str = pf.body->accept(printer) + "\n";
    result += ir_str + "------------------------------";
    return result;
  };

  auto dump_string_frag = [](const StringFragment& sf) -> std::string {
    auto result =
      std::format("frag string: {}, value: \"{}\"\n", sf.label.str(), sf.lit);
    result += "------------------------------";
    return result;
  };

  return std::visit(overloads{dump_proc_frag, dump_string_frag}, f);
}

} // namespace ir