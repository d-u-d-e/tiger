#pragma once
#include "ir/level.hpp"
#include "ir/translator.hpp"
#include "ir/tree.hpp"
#include "parser/ast.hpp"
#include "semant/entry.hpp"
#include "semant/env.hpp"
#include "semant/types.hpp"
#include "semant/visitor.hpp"
#include "string_table.hpp"
#include "symbol.hpp"
#include <cassert>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <variant>

namespace semant
{
class Exception : public std::runtime_error
{
  public:
  Exception(const std::string& what)
    : std::runtime_error(what)
  { }
};

template <typename FrameT>
class Analyzer : TypeCheckerExprVisitor,
                 TypeCheckerDeclVisitor,
                 TypeCheckerVarVisitor,
                 TypeCheckerTypeVisitor
{
  public:
  Analyzer(const std::filesystem::path& filename,
           StringTable& string_table,
           ir::Translator<FrameT>& translator);
  ir::Exp type_check(const parser::ast::Expression& exp);

  private:
  static inline auto int_type = std::make_shared<Integer>();
  static inline auto string_type = std::make_shared<String>();
  static inline auto nil_type = std::make_shared<Nil>();
  static inline auto unit_type = std::make_shared<Unit>();
  using LevelT = Level<FrameT>;

  std::shared_ptr<LevelT> current_level{};
  Environment<TEntry> tenv;
  Environment<VEntry<FrameT>> venv;

  struct CurrentLoop
  {
    std::optional<TempGen::Label> lbreak{}; // where we should jump to when we break inside a loop
    LevelT* level{}; // the level of the function where the loop resides
  };
  CurrentLoop current_loop{};

  private:
  void add_predefined_types();
  void add_predefined_functions();
  template <typename... Args>
  void add_predef_func(const Symbol& s, const SharedType& ret, Args&&... formals);
  void error_at(const lexer::Position& pos, const std::string& err_msg);
  void detect_cycles(const parser::ast::TypeDecl& decl);

  template <typename T>
  bool is_type(const SharedType& t)
  {
    auto& r = *t;
    return typeid(r) == typeid(T);
  }

  bool same_types(const SharedType& t1, const SharedType& t2)
  {
    auto& v1 = *t1;
    auto& v2 = *t2;

    if(typeid(v1) == typeid(v2) && is_type<FunctionType>(t1))
    {
      // for function types we actually need to examine all args and the return type
      return same_function_types(static_cast<FunctionType&>(v1), static_cast<FunctionType&>(v2));
    }
    else
    {
      return t1 == t2;
    }
  }

  bool same_function_types(const FunctionType& t1, const FunctionType& t2)
  {
    if(!same_types(t1.ret, t2.ret))
    {
      return false;
    }
    auto& formals1 = t1.formals;
    auto& formals2 = t2.formals;

    if(formals1.size() != formals2.size())
    {
      return false;
    }

    bool same{true};
    for(size_t i = 0; i < formals1.size() && same; i++)
    {
      same &= (same_types(formals1[i], formals2[i]));
    }
    return same;
  }

  bool can_assign(const SharedType& tlhs, const SharedType& trhs)
  {
    if(is_type<Record>(tlhs) && is_type<Nil>(trhs))
    {
      return true;
    }
    return same_types(tlhs, trhs);
  }

  SharedType skip_name_types(const SharedType& t)
  {
    // the exit guarantee follows in case there are no cycles
    SharedType r = t;
    while(is_type<Name>(r))
    {
      r = (*tenv.lookup(dynamic_cast<const Name*>(r.get())->name)).t;
    }
    return r;
  }

  types::Result visit_string_exp(const parser::ast::StringExp& exp) override
  {
    return Result{string_type, translator.string(exp.value)};
  }

  types::Result visit_assign_exp(const parser::ast::AssignExp& exp) override
  {
    auto tvar = exp.var->accept(*this);
    auto trhs = exp.exp->accept(*this);

    if(!can_assign(tvar.type, trhs.type))
    {
      error_at(
        exp.position,
        std::format("cannot assign '{}' to '{}'", to_string(trhs.type), to_string(tvar.type)));
    }
    return Result{unit_type, translator.assign(std::move(tvar.ir), std::move(trhs.ir))};
  }

  types::Result visit_op_exp(const parser::ast::OpExp& exp) override
  {
    auto tlhs = exp.left->accept(*this);
    auto trhs = exp.right->accept(*this);

    switch(exp.op)
    {
    case parser::ast::Operator::plus:
    case parser::ast::Operator::minus:
    case parser::ast::Operator::times:
    case parser::ast::Operator::divide: {
      // these can be applied to integers
      if(!is_type<Integer>(tlhs.type) || !is_type<Integer>(trhs.type))
      {
        error_at(exp.position, "invalid operand types");
      }
      return Result{int_type,
                    translator.binary_exp(exp.op, std::move(tlhs.ir), std::move(trhs.ir))};
      break;
    }
    case parser::ast::Operator::less:
    case parser::ast::Operator::less_equal:
    case parser::ast::Operator::greater:
    case parser::ast::Operator::greater_equal: {
      // these can be applied to integers
      if(!is_type<Integer>(tlhs.type) || !is_type<Integer>(trhs.type))
      {
        error_at(exp.position, "invalid operand types");
      }
      return Result{int_type, translator.rel_exp(exp.op, std::move(tlhs.ir), std::move(trhs.ir))};
      break;
    }
    case parser::ast::Operator::equal:
    case parser::ast::Operator::not_equal: {
      // these can be applied to integers, strings, records and arrays
      if(is_type<String>(tlhs.type) && is_type<String>(trhs.type))
      {
        return Result{int_type,
                      (exp.op == parser::ast::Operator::equal)
                        ? translator.strings_equal(std::move(tlhs.ir), std::move(trhs.ir))
                        : translator.strings_nequal(std::move(tlhs.ir), std::move(trhs.ir))};
      }
      else if((is_type<Record>(tlhs.type) && is_type<Nil>(trhs.type)) ||
              (is_type<Record>(trhs.type) && is_type<Nil>(tlhs.type)) ||
              (same_types(tlhs.type, trhs.type) &&
               (is_type<Integer>(tlhs.type) || is_type<Array>(tlhs.type) ||
                is_type<Record>(tlhs.type))))
      {
        return Result{int_type, translator.rel_exp(exp.op, std::move(tlhs.ir), std::move(trhs.ir))};
      }
      error_at(exp.position, "invalid operand types");
    }
    default:
      break;
    }
    assert(false);
    std::unreachable();
  }

  types::Result visit_int_exp(const parser::ast::IntExp& exp) override
  {
    return Result{int_type, translator.constant(exp.value)};
  }

  types::Result visit_var_exp(const parser::ast::VarExp& exp) override
  {
    return exp.var->accept(*this);
  }

  types::Result visit_seq_exp(const parser::ast::SeqExp& exp) override
  {
    std::vector<ir::Exp> exps;
    SharedType tres{unit_type};
    for(auto& [e, pos] : exp.exps)
    {
      auto [type, ir] = e->accept(*this);
      tres = type;
      exps.push_back(std::move(ir));
    }
    return {tres, translator.seq_exp(std::move(exps))};
  }

  types::Result visit_array_exp(const parser::ast::ArrayExp& exp) override
  {
    auto rsize = exp.size->accept(*this);
    auto rinit = exp.init->accept(*this);
    auto texpr = tenv.lookup(exp.type);

    if(!texpr || !is_type<Array>(texpr->t))
    {
      error_at(exp.position, std::format("undeclared array type '{}'", exp.type.str()));
    }
    else if(!is_type<Integer>(rsize.type))
    {
      // TODO: it must be positive!
      error_at(exp.position, "array size must be an integer");
    }
    else
    {
      auto& arr = dynamic_cast<Array&>(*texpr->t);
      if(!can_assign(skip_name_types(arr.type), rinit.type))
      {
        error_at(exp.position,
                 std::format("array type mismatch: '{}' != '{}'",
                             to_string(arr.type),
                             to_string(rinit.type)));
      }
    }
    // this is an array type, whose elements may be name types
    return {texpr->t, translator.array_exp(std::move(rsize.ir), std::move(rinit.ir))};
  }

  types::Result visit_nil_exp([[maybe_unused]] const parser::ast::NilExp& exp) override
  {
    return Result{nil_type, translator.constant(0)};
  }

  types::Result visit_record_exp(const parser::ast::RecordExp& exp) override
  {
    auto maybe_rec = tenv.lookup(exp.type);
    if(!maybe_rec || !is_type<Record>(maybe_rec->t))
    {
      error_at(exp.position, std::format("undeclared record type '{}'", exp.type.str()));
    }
    auto& trec = dynamic_cast<Record&>(*maybe_rec->t);

    auto rsize = trec.fields.size();
    auto esize = exp.fields.size();

    if(rsize != esize)
    {
      error_at(exp.position, std::format("expected {} fields, got {}", rsize, esize));
    }

    // typecheck record fields
    std::vector<ir::Exp> fields;
    for(size_t i = 0; i < rsize; i++)
    {
      auto& formal = trec.fields[i];
      auto& actual = exp.fields[i];

      if(formal.first != actual.name)
      {
        error_at(
          actual.position,
          std::format("expected field '{}', got '{}'", formal.first.str(), actual.name.str()));
      }

      // note that trec->fields[i] could be a name type
      // this can occur while type checking mutually recursive types
      auto tactual = actual.exp->accept(*this);
      auto tformal = skip_name_types(formal.second);
      if(!can_assign(tformal, tactual.type))
      {
        error_at(actual.position,
                 std::format("expected type '{}' for field '{}', got '{}'",
                             to_string(formal.second),
                             actual.name.str(),
                             to_string(tactual.type)));
      }
      fields.emplace_back(std::move(tactual.ir));
    }
    return Result{maybe_rec->t, translator.record_exp(std::move(fields))};
  }

  types::Result visit_if_exp(const parser::ast::IfExp& exp) override
  {
    Result r{};
    auto tcond = exp.cond->accept(*this);
    if(!is_type<Integer>(tcond.type))
    {
      error_at(exp.position, "the condition must be an integer");
    }
    auto tthen = exp.then->accept(*this);
    r.type = unit_type;

    if(exp.else_)
    {
      r.type = tthen.type;
      auto telse = exp.else_->accept(*this);
      if(!same_types(tthen.type, telse.type))
      {
        if(is_type<Record>(tthen.type) && is_type<Nil>(telse.type))
        {
          r.type = tthen.type;
        }
        else if(is_type<Record>(telse.type) && is_type<Nil>(tthen.type))
        {
          r.type = telse.type;
        }
        else
        {
          error_at(exp.position, "types of then and else branches must match");
        }
      }
      r.ir =
        translator.if_then_else_exp(std::move(tcond.ir), std::move(tthen.ir), std::move(telse.ir));
      return r;
    }
    else
    {
      if(!is_type<Unit>(tthen.type))
      {
        error_at(exp.position, "the then branch must not produce any value");
      }
    }

    r.ir = translator.if_then_exp(std::move(tcond.ir), std::move(tthen.ir));
    return r;
  }

  types::Result visit_break_exp(const parser::ast::BreakExp& exp) override
  {
    // a break in a procedure p cannot terminate a loop in procedure q, even if p is nested within q
    if(!current_loop.lbreak.has_value() || current_level.get() != current_loop.level)
    {
      error_at(exp.position, "break statement not within a loop");
    }
    return Result{unit_type, translator.break_exp(*current_loop.lbreak)};
  }

  types::Result visit_while_exp(const parser::ast::WhileExp& exp) override
  {
    // condition must be an integer
    auto rcond = exp.cond->accept(*this);
    if(!is_type<Integer>(rcond.type))
    {
      error_at(exp.position, "the condition must be an integer");
    }

    auto current_loop_saved = current_loop;
    auto breakl = TempGen::new_label();
    current_loop.lbreak = breakl;
    current_loop.level = current_level.get();

    // body must not produce any value
    auto rbody = exp.body->accept(*this);
    if(!is_type<Unit>(rbody.type))
    {
      error_at(exp.position, "the body of the while loop must not produce any value");
    }
    current_loop = current_loop_saved;

    return Result{unit_type,
                  translator.while_exp(std::move(rcond.ir), std::move(rbody.ir), breakl)};
  }

  types::Result visit_for_exp(const parser::ast::ForExp& exp) override
  {
    // high and low must be integers
    auto rlow = exp.low->accept(*this);
    auto rhigh = exp.high->accept(*this);

    if(!is_type<Integer>(rlow.type))
    {
      error_at(exp.position, "the lower bound must be an integer");
    }
    else if(!is_type<Integer>(rhigh.type))
    {
      error_at(exp.position, "the upper bound must be an integer");
    }

    auto current_loop_saved = current_loop;
    auto breakl = TempGen::new_label();
    current_loop.lbreak = breakl;
    current_loop.level = current_level.get();

    venv.begin_scope();
    auto access = translator.alloc_local(*current_level, *exp.escape);
    venv.enter(exp.var, SimpleVarEntry<FrameT>(int_type, access));
    auto rbody = exp.body->accept(*this);
    venv.end_scope();
    current_loop = current_loop_saved;

    if(!is_type<Unit>(rbody.type))
    {
      error_at(exp.position, "the body of the for loop must not produce any value");
    }

    return Result{unit_type,
                  translator.for_exp(
                    access, std::move(rlow.ir), std::move(rhigh.ir), std::move(rbody.ir), breakl)};
  }

  types::Result visit_call_exp(const parser::ast::CallExp& exp) override
  {
    types::Result callee_result = exp.callee->accept(*this);

    if(!is_type<FunctionType>(callee_result.type))
    {
      error_at(exp.position, "expression is not callable");
    }

    std::vector<ir::Exp> arg_exps;
    auto& function_type = dynamic_cast<FunctionType&>(*callee_result.type);

    // check the arguments
    auto fsize = function_type.formals.size();
    auto asize = exp.args.size();

    if(asize != fsize)
    {
      error_at(exp.position, std::format("expected {} arguments, got {}", fsize, asize));
    }

    for(size_t i = 0; i < asize; i++)
    {
      auto [tactual, ir] = exp.args[i]->accept(*this);
      auto texpected = function_type.formals[i];
      if(!same_types(skip_name_types(texpected), tactual))
      {
        error_at(exp.position,
                 std::format("argument {} expects type '{}', got '{}'",
                             i,
                             texpected->to_string(),
                             tactual->to_string()));
      }
      arg_exps.emplace_back(std::move(ir));
    };

    return Result{function_type.ret,
                  translator.call_exp(std::move(callee_result.ir), std::move(arg_exps))};
  }

  types::Result visit_let_exp(const parser::ast::LetExp& exp) override
  {
    tenv.begin_scope();
    venv.begin_scope();

    std::vector<ir::Exp> exp_list;
    for(auto& decl : exp.decls)
    {
      auto r = decl->accept(*this);
      if(!std::holds_alternative<std::monostate>(r.ir))
      {
        // there is some code here to be put before the body
        exp_list.emplace_back(std::move(r.ir));
      }
    }

    auto res = exp.body->accept(*this);
    exp_list.emplace_back(std::move(res.ir));
    res.ir = translator.seq_exp(std::move(exp_list));

    venv.end_scope();
    tenv.end_scope();
    return res;
  }

  types::Result visit_func_decl(const parser::ast::FuncDecl& decl) override
  {
    /*
    To handle mutually recursive functions:

    function is_even(n: int): int = if n = 0 then 1 else is_odd(n-1)
    function is_odd(n: int): int = if n = 0 then 0 else is_even(n-1) 

    We first augment the venv with the function headers:

    is_even -> FuncEntry(formals=[int], result=int)
    is_odd  -> FuncEntry(formals=[int], result=int)

    So that type checking the body can proceed without undefined references.

  */
    std::unordered_set<Symbol::Identifier> batch;
    std::vector<ir::Exp> exp_list; // code generated by closures

    for(auto& fdecl : decl.decls)
    {
      if(batch.contains(fdecl->name.id()))
      {
        error_at(fdecl->position, std::format("redeclaration of function '{}'", fdecl->name.str()));
      }
      batch.insert(fdecl->name.id());

      // type check the parameters
      std::vector<bool> escapes;
      std::vector<SharedType> formals;
      for(auto& param : fdecl->params)
      {
        auto tparam = tenv.lookup(param.type);
        if(!tparam)
        {
          error_at(param.position, std::format("undeclared parameter type '{}'", param.type.str()));
        }
        formals.push_back(tparam->t);
        escapes.push_back(*param.escape);
      }

      // typecheck return type (not against expression)
      SharedType tresult = unit_type;
      if(fdecl->result)
      {
        auto fdecl_result = fdecl->result.value();
        auto opt_tresult = tenv.lookup(fdecl_result.first);
        if(!opt_tresult)
        {
          error_at(fdecl_result.second,
                   std::format("undeclared return type '{}'", fdecl_result.first.str()));
        }
        tresult = opt_tresult->t;
      }

      // we create the closure entry in the environment and create the code that generates the closure
      auto flabel = TempGen::new_label();
      auto closure_init = translator.make_closure(flabel, current_level.get());
      typename LevelT::Access ax = translator.alloc_local(*current_level, *fdecl->escape);
      exp_list.push_back(
        translator.assign(translator.var(ax, current_level.get()), std::move(closure_init)));

      // we add the function headers for mutually recursive functions
      venv.enter(fdecl->name,
                 ClosureEntry<FrameT>(flabel,
                                      std::make_shared<FunctionType>(formals, tresult),
                                      translator.new_level(current_level.get(), flabel, escapes),
                                      ax));
    }

    // go through the bodies
    for(auto& fdecl : decl.decls)
    {
      ClosureEntry<FrameT> closure_entry =
        std::get<ClosureEntry<FrameT>>(venv.lookup(fdecl->name)->v);
      venv.begin_scope(); // body scope augmented with formals

      // add formals
      auto ax = translator.formals(*closure_entry.level);
      for(size_t i = 0; i < fdecl->params.size(); i++)
      {
        auto& param = fdecl->params[i];
        venv.enter(param.name, SimpleVarEntry<FrameT>(tenv.lookup(param.type)->t, ax[i]));
      }

      // type check return type
      auto prev_level = current_level;
      current_level = closure_entry.level;
      auto rbody = fdecl->body->accept(*this);
      current_level = prev_level;

      if(!same_types(skip_name_types(closure_entry.fun_type->ret), rbody.type))
      {
        auto pos = fdecl->position;
        if(fdecl->result)
        {
          // use the position of the return type
          pos = fdecl->result.value().second;
        }
        error_at(pos,
                 std::format("return type '{}' does not match body type '{}'",
                             to_string(closure_entry.fun_type->ret),
                             to_string(rbody.type)));
      }

      translator.proc_entry_exit(closure_entry.level, std::move(rbody.ir));
      venv.end_scope(); // end body scope
    }

    // This decl has no type
    return Result{.type = {}, .ir = translator.seq_exp(std::move(exp_list))};
  }

  types::Result visit_var_decl(const parser::ast::VarDecl& decl) override
  {
    auto tinit = decl.init->accept(*this);
    typename LevelT::Access ax = translator.alloc_local(*current_level, *decl.escape);

    if(decl.type)
    {
      auto tpos = decl.type.value().second;
      auto tname = decl.type.value().first;
      auto tdecl = tenv.lookup(tname);

      if(!tdecl)
      {
        error_at(tpos, std::format("undeclared type '{}'", tname.str()));
      }
      if(!can_assign(skip_name_types(tdecl->t), tinit.type))
      {
        error_at(tpos,
                 std::format("decl type '{}' does not match expr type '{}'",
                             tname.str(),
                             to_string(tinit.type)));
      }
      venv.enter(decl.name, SimpleVarEntry<FrameT>(tdecl->t, ax));
    }
    else
    {
      if(is_type<Nil>(tinit.type))
      {
        // Nil must be constrained by a record type
        error_at(decl.position, "nil must be constrained by a record type");
      }
      venv.enter(decl.name, SimpleVarEntry<FrameT>(tinit.type, ax));
    }

    return Result{nullptr,
                  translator.assign(translator.var(ax, current_level.get()), std::move(tinit.ir))};
  }

  types::Result visit_type_decl(const parser::ast::TypeDecl& decl) override
  {
    std::unordered_set<Symbol::Identifier> batch;

    // add the headers to the type environment
    for(auto& tdecl : decl.decls)
    {
      // we register the symbol as a name type, to be resolved in a later pass
      // this way it exists in the environment
      if(batch.contains(tdecl->name.id()))
      {
        error_at(tdecl->position, std::format("redeclaration of type '{}'", tdecl->name.str()));
      }
      tenv.enter(tdecl->name, TEntry{std::make_shared<Name>(tdecl->name, nullptr)});
      batch.insert(tdecl->name.id());
    }

    // next we replace all those fake names with the true type
    for(auto& tdecl : decl.decls)
    {
      auto actual = tdecl->type->accept(*this);
      tenv.replace(tdecl->name, TEntry{actual});
    }

    // prevent cycles
    detect_cycles(decl);
    return Result{}; // does not generate code nor type for caller
  }

  types::SharedType visit_name_type(const parser::ast::NameType& type) override
  {
    auto ty = tenv.lookup(type.name);
    if(!ty)
    {
      error_at(type.position, std::format("undeclared type '{}'", type.name.str()));
    }
    return ty->t;
  }

  types::SharedType visit_array_type(const parser::ast::ArrayType& type) override
  {
    auto elem_type = tenv.lookup(type.name);
    if(!elem_type)
    {
      error_at(type.position, std::format("undeclared type '{}'", type.name.str()));
    }
    return std::make_shared<Array>(elem_type->t);
  }

  types::SharedType visit_func_type(const parser::ast::FunctionType& type) override
  {
    std::vector<SharedType> args;
    SharedType ret{};

    for(auto& arg : type.arg_types)
    {
      args.push_back(arg->accept(*this));
    }
    ret = type.ret_type->accept(*this);

    return std::make_shared<FunctionType>(std::move(args), std::move(ret));
  }

  types::SharedType visit_record_type(const parser::ast::RecordType& type) override
  {
    std::vector<std::pair<Symbol, SharedType>> fields;
    for(auto& field : type.fields)
    {
      auto tfield = tenv.lookup(field.type);
      if(!tfield)
      {
        error_at(field.position, std::format("undeclared type '{}'", field.type.str()));
      }
      fields.push_back({field.name, tfield->t});
    }
    return std::make_shared<Record>(fields);
  }

  types::Result visit_var(const parser::ast::Var& var) override
  {
    const VEntry<FrameT>* maybe_var = venv.lookup(var.name);
    if(!maybe_var)
    {
      error_at(var.position, std::format("undeclared identifier '{}'", var.name.str()));
    }

    if(std::holds_alternative<SimpleVarEntry<FrameT>>(maybe_var->v))
    {
      auto& entry = std::get<SimpleVarEntry<FrameT>>(maybe_var->v);
      return Result{skip_name_types(entry.type), translator.var(entry.access, current_level.get())};
    }
    assert(std::holds_alternative<ClosureEntry<FrameT>>(maybe_var->v));
    ClosureEntry<FrameT> entry = std::get<ClosureEntry<FrameT>>(maybe_var->v);

    if(entry.level == translator.outermost_level())
    {
      return Result{entry.fun_type,
                    translator.make_closure(entry.label, translator.outermost_level().get())};
    }

    // Otherwise the closure has been created upon function definition, and this is available at entry.access in
    // the current frame or in other frames if it escapes
    return Result{entry.fun_type, translator.var(entry.access, current_level.get())};
  }

  types::Result visit_field_var(const parser::ast::FieldVar& var) override
  {
    auto tlhs = var.var->accept(*this);
    // . applicable to records only
    if(!is_type<Record>(tlhs.type))
    {
      error_at(var.position, std::format("'{}' is not a record type", to_string(tlhs.type)));
    }
    auto& record = dynamic_cast<Record&>(*tlhs.type);

    // check whether the field name belongs to the record fields
    size_t i = 0;
    auto size = record.fields.size();
    for(; i < size; i++)
    {
      if(std::get<0>(record.fields[i]) == var.name)
      {
        break;
      }
    }

    if(i == size)
    {
      error_at(var.position, std::format("unexpected record field name '{}'", var.name.str()));
    }
    return Result{skip_name_types(record.fields[i].second),
                  translator.record_field(std::move(tlhs.ir), i)};
  }

  types::Result visit_subscript_var(const parser::ast::SubscriptVar& var) override
  {
    // [] applicable to arrays only
    auto lhs = var.var->accept(*this);
    if(!is_type<Array>(lhs.type))
    {
      error_at(var.position, std::format("'{}' is not an array type", to_string(lhs.type)));
    }

    auto& array = dynamic_cast<Array&>(*lhs.type);
    // expression must be an integer
    auto rexp = var.exp->accept(*this);
    if(!is_type<Integer>(rexp.type))
    {
      error_at(var.position, "expression between '[]' must be an integer");
    }

    // the type of the expression is the type of each array element
    return Result{skip_name_types(array.type),
                  translator.array_subscript(std::move(lhs.ir), std::move(rexp.ir))};
  }

  private:
  StringTable& string_table;
  ir::Translator<FrameT>& translator;
  std::string filename; // for error reporting only
};

// implementations
template <typename FrameT>
Analyzer<FrameT>::Analyzer(const std::filesystem::path& filename,
                           StringTable& string_table,
                           ir::Translator<FrameT>& translator)
  : string_table(string_table)
  , translator(translator)
  , filename(filename)
{
  add_predefined_types();
  add_predefined_functions();

  // current level is where the main program lives
  current_level = translator.main_level();
}

template <typename FrameT>
ir::Exp Analyzer<FrameT>::type_check(const parser::ast::Expression& exp)
{
  auto t = exp.accept(*this);
  return std::move(t.ir);
}

template <typename FrameT>
void Analyzer<FrameT>::add_predefined_types()
{
  // predefined types
  tenv.enter(string_table.symbol("int"), TEntry{int_type});
  tenv.enter(string_table.symbol("string"), TEntry{string_type});
  // nil is not really a type, but it is convenient to consider it as such
  tenv.enter(string_table.symbol("nil"), TEntry{nil_type});
}

template <typename FrameT>
template <typename... Args>
void Analyzer<FrameT>::add_predef_func(const Symbol& s, const SharedType& ret, Args&&... formals)
{
  venv.enter(s,
             ClosureEntry(TempGen::named_label(s.str()),
                          std::make_shared<FunctionType>(
                            std::vector<SharedType>{std::forward<Args>(formals)...}, ret),
                          translator.outermost_level(),
                          {}));
}

template <typename FrameT>
void Analyzer<FrameT>::add_predefined_functions()
{
  add_predef_func(string_table.symbol("print"), unit_type, string_type);
  add_predef_func(string_table.symbol("flush"), unit_type);
  add_predef_func(string_table.symbol("getchr"), string_type);
  add_predef_func(string_table.symbol("ord"), int_type, string_type);
  add_predef_func(string_table.symbol("chr"), string_type, int_type);
  add_predef_func(string_table.symbol("size"), int_type, string_type);
  add_predef_func(string_table.symbol("substring"), string_type, string_type, int_type, int_type);
  add_predef_func(string_table.symbol("concat"), string_type, string_type, string_type);
  add_predef_func(string_table.symbol("not"), int_type, int_type);
  add_predef_func(string_table.symbol("exit"), unit_type, int_type);
}

template <typename FrameT>
void Analyzer<FrameT>::error_at(const lexer::Position& pos, const std::string& err_msg)
{
  throw Exception(std::format("[{}:{}:{}] Err: {}", filename, pos.line, pos.column, err_msg));
}

template <typename FrameT>
void Analyzer<FrameT>::detect_cycles(const parser::ast::TypeDecl& decl)
{
  /*
    example 1:
    type A = B
    type B = C
    type C = B

    tenv after having parsed the "headers"
    "A" -> @1: Name("A", @0)
    "B" -> @2: Name("B", @0)
    "C" -> @3: Name("C", @0)

    tenv after having parsed the "bodies"
    "A" -> @2
    "B" -> @3
    "C" -> @3

    example 2:
    type A = B
    type B = array of A

    tenv after having parsed the "headers"
    "A" -> @1: Name("A", @0)
    "B" -> @2: Name("B", @0)

    tenv after having parsed the "bodies"
    "A" -> @2
    "B" -> @3: Array(@2)

    example 3:
    type A = A

    tenv after having parsed the "headers"
    "A" -> @1: Name("A", @0)

    tenv after having parsed the "bodies"
    "A" -> @1

    example 4:
    type A = B
    type B = int

    tenv after having parsed the "headers"
    "A" -> @1: Name("A", @0)
    "B" -> @2: int

    tenv after having parsed the "bodies"
    "A" -> @2
    "B" -> @2

    example 5:
    type A = B
    type B = C
    type C = A

    tenv after having parsed the "headers"
    "A" -> @1: Name("A", @0)
    "B" -> @2: Name("B", @0)
    "C" -> @3: Name("C", @0)

    tenv after having parsed the "bodies"
    "A" -> @2
    "B" -> @3
    "C" -> @2
  */

  // this can be made more efficient
  std::unordered_set<SharedType> visited;
  for(auto& tdecl : decl.decls)
  {
    visited.clear();
    auto actual = tenv.lookup(tdecl->name)->t;

    // chase the sequence until a record or cycle is found
    while(true)
    {

      if(visited.contains(actual))
      {
        error_at(tdecl->position, "cycle in type declaration");
      }
      else
      {
        visited.insert(actual);
      }

      if(is_type<Name>(actual))
      {
        actual = tenv.lookup((dynamic_cast<Name*>(actual.get()))->name)->t;
      }
      else if(is_type<Array>(actual))
      {
        actual = dynamic_cast<Array*>(actual.get())->type;
      }
      else
      {
        break;
      }
    }
  }
}

} // namespace semant