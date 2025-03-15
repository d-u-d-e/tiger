#include <iostream>
#include <seman/analyzer.hpp>
#include <unordered_set>

namespace seman
{

static auto int_type = std::make_shared<Integer>();
static auto string_type = std::make_shared<String>();
static auto nil_type = std::make_shared<Nil>();
static auto unit_type = std::make_shared<Unit>();

Analyzer::Analyzer(symbol::StringTable& string_table,
                   ir::Translator& translator)
  : string_table(string_table)
  , translator(translator)
  , prev_level(translator.outermost_level())
{
  add_predefined_types();
  add_predefined_functions();

  // create the current level, e.g. where the main program lives
  // TODO: formal arguments?
  current_level =
    translator.new_level(*prev_level, ir::Temp::named_label("main"), {});
}

void Analyzer::add_predefined_types()
{
  // predefined types
  tenv.enter(string_table.symbol("int"), env::TEntry{int_type});
  tenv.enter(string_table.symbol("string"), env::TEntry{string_type});
  // nil is not really a type, but it is convenient to consider it as such
  tenv.enter(string_table.symbol("nil"), env::TEntry{nil_type});
}

template <typename... Args>
void Analyzer::add_predef_func(const symbol::Symbol& s,
                               const shared_type_t& ret,
                               Args&&... formals)
{
  constexpr auto fsize = sizeof...(Args);
  auto l = translator.new_level(*translator.outermost_level(),
                                ir::Temp::new_label(),
                                std::vector<bool>(fsize, false));
  venv.enter(
    s,
    env::FuncEntry(std::vector<shared_type_t>{std::forward<Args>(formals)...},
              ret,
              std::move(l)));
}

void Analyzer::add_predefined_functions()
{
  add_predef_func(string_table.symbol("print"), unit_type, string_type);
  add_predef_func(string_table.symbol("flush"), unit_type);
  add_predef_func(string_table.symbol("getchar"), string_type);
  add_predef_func(string_table.symbol("ord"), int_type, string_type);
  add_predef_func(string_table.symbol("chr"), string_type, int_type);
  add_predef_func(string_table.symbol("size"), int_type, string_type);
  add_predef_func(string_table.symbol("substring"),
                  string_type,
                  string_type,
                  int_type,
                  int_type);
  add_predef_func(
    string_table.symbol("concat"), string_type, string_type, string_type);
  add_predef_func(string_table.symbol("not"), int_type, int_type);
  add_predef_func(string_table.symbol("exit"), unit_type, int_type);
}

void Analyzer::error_at(const lexer::Position& pos, const std::string& err_msg)
{
  throw std::runtime_error(
    std::format("[line {}:{}] Err: {}", pos.line, pos.column, err_msg));
}

void Analyzer::type_check(const parser::ast::Expression& exp)
{
  auto t = exp.accept(*this);
  // TODO: do something with t
}

shared_type_t Analyzer::visit_string_exp(const parser::ast::StringExp& exp)
{
  return string_type;
};

shared_type_t Analyzer::visit_assign_exp(const parser::ast::AssignExp& exp)
{
  auto tvar = exp.var->accept(*this);
  auto trhs = exp.exp->accept(*this);

  if(!can_assign(tvar, trhs)) {
    error_at(exp.position,
             std::format(
               "cannot assign '{}' to '{}'", to_string(trhs), to_string(tvar)));
  }
  return unit_type;
};

template <typename T>
bool Analyzer::is_type(const shared_type_t& t)
{
  return typeid(*t) == typeid(T);
}

bool Analyzer::can_assign(const shared_type_t& tlhs, const shared_type_t& trhs)
{
  if(is_type<Record>(tlhs) && is_type<Nil>(trhs)) {
    return true;
  }
  return same_types(tlhs, trhs);
}

shared_type_t Analyzer::skip_name_types(const shared_type_t& t)
{
  // the exit guarantee follows in case there are no cycles
  shared_type_t r = t;
  while(is_type<Name>(r)) {
    r = (*tenv.lookup(dynamic_cast<const Name*>(r.get())->name)).t;
  }
  return r;
}

shared_type_t Analyzer::visit_op_exp(const parser::ast::OpExp& exp)
{
  auto tlhs = exp.left->accept(*this);
  auto trhs = exp.right->accept(*this);

  switch(exp.op) {
  case parser::ast::Operator::plus:
  case parser::ast::Operator::minus:
  case parser::ast::Operator::times:
  case parser::ast::Operator::divide: {
    // these can be applied to integers
    if(!same_types(tlhs, trhs) || !is_type<Integer>(tlhs)) {
      error_at(exp.position, "operands must be integers");
    }
  }
  case parser::ast::Operator::equal:
  case parser::ast::Operator::not_equal: {
    // these can be applied to integers, strings, records and arrays
    if(!same_types(tlhs, trhs)) {
      // testing equality between nil and record is supported
      if(is_type<Nil>(tlhs) && is_type<Record>(trhs)) {
        return int_type;
      }
      else if(is_type<Nil>(trhs) && is_type<Record>(tlhs)) {
        return int_type;
      }
      error_at(exp.position, "operands must be of the same type");
    }
    else if(is_type<Integer>(tlhs) || is_type<String>(tlhs) ||
            is_type<Array>(tlhs) || is_type<Record>(tlhs)) {
      return int_type;
    }
    error_at(exp.position,
             "operands must be integers, strings, records or arrays");
    break;
  }

  case parser::ast::Operator::less:
  case parser::ast::Operator::less_equal:
  case parser::ast::Operator::greater:
  case parser::ast::Operator::greater_equal: {
    // these can be applied to integers or strings
    if(!same_types(tlhs, trhs) ||
       (!is_type<Integer>(tlhs) && !is_type<String>(tlhs))) {
      error_at(exp.position, "operands must be integers or strings");
    }
    break;
  }
  default:
    assert(false);
  }
  return int_type;
};

shared_type_t Analyzer::visit_int_exp(const parser::ast::IntExp& exp)
{
  return int_type;
};

shared_type_t Analyzer::visit_var_exp(const parser::ast::VarExp& exp)
{
  return exp.var->accept(*this);
};

shared_type_t Analyzer::visit_seq_exp(const parser::ast::SeqExp& exp)
{
  shared_type_t tres = unit_type;
  for(auto& [e, pos] : exp.exps) {
    tres = e->accept(*this);
  }
  return tres;
};

shared_type_t Analyzer::visit_array_exp(const parser::ast::ArrayExp& exp)
{
  auto tsize = exp.size->accept(*this);
  auto tinit = exp.init->accept(*this);
  auto texpr = tenv.lookup(exp.type);

  if(!texpr || !is_type<Array>(texpr->t)) {
    error_at(exp.position,
             std::format("undefined array type '{}'", exp.type.str()));
  }
  else if(!is_type<Integer>(tsize)) {
    error_at(exp.position, "array size must be an integer");
  }
  else {
    auto arr = dynamic_cast<Array*>(texpr->t.get());
    if(!can_assign(skip_name_types(arr->type), tinit)) {
      error_at(exp.position,
               std::format("array type mismatch: '{}' != '{}'",
                           to_string(arr->type),
                           to_string(tinit)));
    }
  }
  // this is an array type, whose elements may be name types
  return texpr->t;
}

shared_type_t Analyzer::visit_nil_exp(const parser::ast::NilExp& exp)
{
  return nil_type;
};

shared_type_t Analyzer::visit_record_exp(const parser::ast::RecordExp& exp)
{
  auto maybe_rec = tenv.lookup(exp.type);
  if(!maybe_rec || !is_type<Record>(maybe_rec->t)) {
    error_at(exp.position,
             std::format("undefined record type '{}'", exp.type.str()));
  }
  auto trec = dynamic_cast<Record*>(maybe_rec->t.get());

  auto rsize = trec->fields.size();
  auto esize = exp.fields.size();

  if(rsize != esize) {
    error_at(exp.position,
             std::format("expected {} fields, got {}", rsize, esize));
  }

  // typecheck record fields
  for(int i = 0; i < rsize; i++) {
    auto& formal = trec->fields[i];
    auto& actual = exp.fields[i];

    if(formal.first != actual.name) {
      error_at(actual.position,
               std::format("expected field '{}', got '{}'",
                           formal.first.str(),
                           actual.name.str()));
    }

    // note that trec->fields[i] could be a name type
    // this can occur while type checking mutually recursive types
    auto tactual = actual.exp->accept(*this);
    auto tformal = skip_name_types(formal.second);
    if(!can_assign(tformal, tactual)) {
      error_at(actual.position,
               std::format("expected type '{}' for field '{}', got '{}'",
                           to_string(formal.second),
                           actual.name.str(),
                           to_string(tactual)));
    }
  }
  return maybe_rec->t;
};

shared_type_t Analyzer::visit_if_exp(const parser::ast::IfExp& exp)
{
  auto tcond = exp.cond->accept(*this);
  if(!is_type<Integer>(tcond)) {
    error_at(exp.position, "the condition must be an integer");
  }
  auto tthen = exp.then->accept(*this);

  if(exp.else_) {
    auto telse = exp.else_->accept(*this);
    if(!same_types(tthen, telse)) {
      if(is_type<Record>(tthen) && is_type<Nil>(telse)) {
        return tthen;
      }
      else if(is_type<Record>(telse) && is_type<Nil>(tthen)) {
        return telse;
      }
      else {
        error_at(exp.position, "types of then and else branches must match");
      }
    }
    return tthen;
  }
  else {
    if(!is_type<Unit>(tthen)) {
      error_at(exp.position, "the then branch must not produce any value");
    }
  }
  return unit_type;
};

shared_type_t Analyzer::visit_break_exp(const parser::ast::BreakExp& exp)
{
  if(!can_break) {
    error_at(exp.position, "break statement not within a loop");
  }
  return unit_type;
};

shared_type_t Analyzer::visit_while_exp(const parser::ast::WhileExp& exp)
{
  // condition must be an integer
  if(!is_type<Integer>(exp.cond->accept(*this))) {
    error_at(exp.position, "the condition must be an integer");
  } // body must not produce any value
  else {
    bool can_break_saved = can_break;
    can_break = true;
    if(!is_type<Unit>(exp.body->accept(*this))) {
      error_at(exp.position,
               "the body of the while loop must not produce any value");
    }
    can_break = can_break_saved;
  }
  return unit_type;
};

shared_type_t Analyzer::visit_for_exp(const parser::ast::ForExp& exp)
{
  // high and low must be integers
  if(!is_type<Integer>(exp.low->accept(*this))) {
    error_at(exp.position, "the lower bound must be an integer");
  }
  else if(!is_type<Integer>(exp.high->accept(*this))) {
    error_at(exp.position, "the upper bound must be an integer");
  } // body must not produce any value
  else {
    bool can_break_saved = can_break;
    can_break = true;
    venv.begin_scope();
    auto access =
      translator.alloc_local(*current_level, true); // TODO: find escape
    venv.enter(exp.var, env::VarEntry(int_type, access));
    auto tbody = exp.body->accept(*this);
    venv.end_scope();
    can_break = can_break_saved;

    if(!is_type<Unit>(tbody)) {
      error_at(exp.position,
               "the body of the for loop must not produce any value");
    }
  }
  return unit_type;
};

shared_type_t Analyzer::visit_call_exp(const parser::ast::CallExp& exp)
{
  auto maybe_fentry = venv.lookup(exp.name);
  if(!maybe_fentry || !std::holds_alternative<env::FuncEntry>(*maybe_fentry)) {
    error_at(exp.position,
             std::format("undefined function '{}'", exp.name.str()));
  }

  // Note: be careful with auto&: calling enter on the env after having
  // obtained a reference to an entry can make it dangling!
  // This is because the env can grow
  auto& fentry = std::get<env::FuncEntry>(*maybe_fentry);

  // check the arguments
  auto fsize = fentry.formals.size();
  auto asize = exp.args.size();

  if(asize != fsize) {
    error_at(exp.position,
             std::format("expected {} arguments, got {}", fsize, asize));
  }

  for(int i = 0; i < asize; i++) {
    auto tactual = exp.args[i]->accept(*this);
    auto texpected = fentry.formals[i];
    if(!same_types(skip_name_types(texpected), tactual)) {
      error_at(exp.position,
               std::format("argument {} expects type '{}', got '{}'",
                           i,
                           texpected->to_string(),
                           tactual->to_string()));
    }
  }
  return skip_name_types(fentry.result);
};

shared_type_t Analyzer::visit_let_exp(const parser::ast::LetExp& exp)
{
  tenv.begin_scope();
  venv.begin_scope();

  for(auto& decl : exp.decls) {
    decl->accept(*this);
  }

  auto tres = exp.body->accept(*this);
  venv.end_scope();
  tenv.end_scope();
  return tres;
};

void Analyzer::visit_func_decl(const parser::ast::FuncDecl& decl)
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
  std::unordered_set<symbol::Identifier> batch;

  for(auto& fdecl : decl.decls) {
    if(batch.contains(fdecl->name.id())) {
      error_at(
        fdecl->position,
        std::format("redeclaration of function '{}'", fdecl->name.str()));
    }
    batch.insert(fdecl->name.id());

    // type check the parameters
    std::vector<shared_type_t> formals;
    for(auto& param : fdecl->params) {
      auto tparam = tenv.lookup(param.type);
      if(!tparam) {
        error_at(
          param.position,
          std::format("undefined parameter type '{}'", param.type.str()));
      }
      formals.push_back(tparam->t);
    }

    // typecheck return type (not against expression)
    shared_type_t tresult = unit_type;
    if(fdecl->result) {
      auto fdecl_result = fdecl->result.value();
      auto opt_tresult = tenv.lookup(fdecl_result.first);
      if(!opt_tresult) {
        error_at(
          fdecl_result.second,
          std::format("undefined return type '{}'", fdecl_result.first.str()));
      }
      tresult = opt_tresult->t;
    }

    // TODO: find escape
    std::vector<bool> escapes(formals.size(), true);
    // add the function header
    venv.enter(
      fdecl->name,
      env::FuncEntry(
        formals,
        tresult,
        translator.new_level(*current_level, ir::Temp::new_label(), escapes)));
  }

  // go through the bodies
  for(auto& fdecl : decl.decls) {
    auto func_entry = std::get<env::FuncEntry>(*venv.lookup(fdecl->name));

    venv.begin_scope(); // body scope augmented with formals

    // add formals
    auto ax = translator.formals(*func_entry.level);
    for(auto i = 0; i < fdecl->params.size(); i++) {
      auto& param = fdecl->params[i];
      venv.enter(param.name, env::VarEntry(tenv.lookup(param.type)->t, ax[i]));
    }

    // type check return type
    auto tbody = fdecl->body->accept(*this);

    if(!same_types(skip_name_types(func_entry.result), tbody)) {

      auto pos = fdecl->position;
      if(fdecl->result) {
        // use the position of the return type
        pos = fdecl->result.value().second;
      }
      error_at(pos,
               std::format("return type '{}' does not match body type '{}'",
                           to_string(func_entry.result),
                           to_string(tbody)));
    }

    venv.end_scope(); // end body scope
  }
};

void Analyzer::visit_var_decl(const parser::ast::VarDecl& decl)
{
  auto tinit = decl.init->accept(*this);
  if(decl.type) {
    auto tpos = decl.type.value().second;
    auto tname = decl.type.value().first;
    auto tdecl = tenv.lookup(tname);

    if(!tdecl) {
      error_at(tpos, std::format("undefined type '{}'", tname.str()));
    }
    if(!can_assign(skip_name_types(tdecl->t), tinit)) {
      error_at(tpos,
               std::format("decl type '{}' does not match expr type '{}'",
                           tname.str(),
                           to_string(tinit)));
    }
    // TODO: find escape
    venv.enter(
      decl.name,
      env::VarEntry(tdecl->t, translator.alloc_local(*current_level, true)));
  }
  else {
    if(is_type<Nil>(tinit)) {
      // Nil must be constrained by a record type
      error_at(decl.position, "nil must be constrained by a record type");
    }
    // TODO: find escape
    venv.enter(
      decl.name,
      env::VarEntry(tinit, translator.alloc_local(*current_level, true)));
  }
};

void Analyzer::visit_type_decl(const parser::ast::TypeDecl& decl)
{
  std::unordered_set<symbol::Identifier> batch;

  // add the headers to the type environment
  for(auto& tdecl : decl.decls) {
    // we register the symbol as a name type, to be resolved in a later pass
    // this way it exists in the environment
    if(batch.contains(tdecl->name.id())) {
      error_at(tdecl->position,
               std::format("redeclaration of type '{}'", tdecl->name.str()));
    }
    tenv.enter(tdecl->name,
               env::TEntry{std::make_shared<Name>(tdecl->name, nullptr)});
    batch.insert(tdecl->name.id());
  }

  // next we replace all those fake names with the true type
  for(auto& tdecl : decl.decls) {
    auto actual = tdecl->type->accept(*this);
    tenv.replace(tdecl->name, env::TEntry{actual});
  }

  // prevent cycles
  detect_cycles(decl);
}

void Analyzer::detect_cycles(const parser::ast::TypeDecl& decl)
{
  /*
    example 1:
    type A = B
    type B = C
    type C = B

    tenv after having parsed the "headers"
    A -> Name("A", 0)
    B -> Name("B", 0)
    C -> Name("C", 0)

    tenv after having parsed the "bodies"
    A' -> Name("B", B)
    B' -> Name("C", C)
    C' -> Name("B", B')

    example 2:
    type A = B
    type B = array of A

    tenv after having parsed the "headers"
    A -> Name("A", 0)
    B -> Name("B", 0)

    tenv after having parsed the "bodies"
    A' -> Name("B", B)
    B' -> Array(A')

    example 3:
    type A = A

    tenv after having parsed the "headers"
    A -> Name("A", 0)

    tenv after having parsed the "bodies"
    A' -> Name("A", A')

    example 4:
    type A = B
    type B = int

    tenv after having parsed the "headers"
    A -> Name("A", 0)
    B -> Name("B", 0)

    tenv after having parsed the "bodies"
    A' -> Name("B", B)
    B' -> int

    example 5:
    type A = B
    type B = C
    type C = A

    tenv after having parsed the "headers"
    A -> Name("A", 0)
    B -> Name("B", 0)
    C -> Name("C", 0)

    tenv after having parsed the "bodies"
    A' -> Name("B", B)
    B' -> Name("C", C)
    C' -> Name("A", A')
  */

  // this can be made more efficient
  std::unordered_set<shared_type_t> visited;
  for(auto& tdecl : decl.decls) {
    visited.clear();
    auto actual = tenv.lookup(tdecl->name)->t;

    // chase the sequence until a record or cycle is found
    while(true) {

      if(visited.contains(actual)) {
        error_at(tdecl->position, "cycle in type declaration");
      }
      else {
        visited.insert(actual);
      }

      if(is_type<Name>(actual)) {
        actual = tenv.lookup((dynamic_cast<Name*>(actual.get()))->name)->t;
      }
      else if(is_type<Array>(actual)) {
        actual = dynamic_cast<Array*>(actual.get())->type;
      }
      else {
        break;
      }
    }
  }
}

shared_type_t Analyzer::visit_name_type(const parser::ast::NameType& type)
{
  auto ty = tenv.lookup(type.name);
  if(!ty) {
    error_at(type.position,
             std::format("undefined type '{}'", type.name.str()));
  }
  return ty->t;
};

shared_type_t Analyzer::visit_array_type(const parser::ast::ArrayType& type)
{
  auto elem_type = tenv.lookup(type.name);
  if(!elem_type) {
    error_at(type.position,
             std::format("undefined type '{}'", type.name.str()));
  }
  return std::make_shared<Array>(elem_type->t);
};

shared_type_t Analyzer::visit_record_type(const parser::ast::RecordType& type)
{
  std::vector<std::pair<symbol::Symbol, shared_type_t>> fields;
  for(auto& field : type.fields) {
    auto tfield = tenv.lookup(field.type);
    if(!tfield) {
      error_at(field.position,
               std::format("undefined type '{}'", field.type.str()));
    }
    fields.push_back({field.name, tfield->t});
  }
  return std::make_shared<Record>(fields);
};

shared_type_t Analyzer::visit_simple_var(const parser::ast::SimpleVar& var)
{
  auto v = venv.lookup(var.name);
  if(!v || !std::holds_alternative<env::VarEntry>(*v)) {
    error_at(var.position,
             std::format("undefined variable '{}'", var.name.str()));
  }
  // TODO: here std::get<VarEntry>(*v).access can be handed back to the translator to generate machine code
  return skip_name_types(std::get<env::VarEntry>(*v).type);
};

shared_type_t Analyzer::visit_field_var(const parser::ast::FieldVar& var)
{
  auto tlhs = var.var->accept(*this);
  // . applicable to records only
  if(!is_type<Record>(tlhs)) {
    error_at(var.position,
             std::format("'{}' is not a record type", to_string(tlhs)));
  }
  auto record = dynamic_cast<Record*>(tlhs.get());

  // check whether the field name belongs to the record fields
  auto iter =
    std::find_if(record->fields.begin(),
                 record->fields.end(),
                 [&var](const auto& p) { return std::get<0>(p) == var.name; });

  if(iter == record->fields.end()) {
    error_at(var.position,
             std::format("unexpected record field name '{}'", var.name.str()));
  }
  return skip_name_types(std::get<1>(*iter));
};

shared_type_t
Analyzer::visit_subscript_var(const parser::ast::SubscriptVar& var)
{
  // [] applicable to arrays only
  auto tlhs = var.var->accept(*this);
  if(!is_type<Array>(tlhs)) {
    error_at(var.position,
             std::format("'{}' is not an array type", to_string(tlhs)));
  }
  auto array = dynamic_cast<Array*>(tlhs.get());

  // expression must be an integer
  auto texp = var.exp->accept(*this);
  if(!is_type<Integer>(texp)) {
    error_at(var.position, "expression between '[]' must be an integer");
  }

  // the type of the expression is the type of each array element
  return skip_name_types(array->type);
};

} // namespace seman