#include <semantic/type_checker.hpp>
#include <unordered_set>

namespace semantic
{

static auto int_type = std::make_shared<types::Integer>();
static auto string_type = std::make_shared<types::String>();
static auto nil_type = std::make_shared<types::Nil>();
static auto unit_type = std::make_shared<types::Unit>();

SemanticAnalyzer::SemanticAnalyzer(symbol::StringTable& string_table)
  : string_table(string_table)
{
  // predefined types
  tenv.enter(string_table.symbol("int"), int_type);
  tenv.enter(string_table.symbol("string"), string_type);
  // nil is not really a type, but it is convenient to consider it as such
  tenv.enter(string_table.symbol("nil"), nil_type);
}

void SemanticAnalyzer::error_at(const lexer::Position& pos,
                           const std::string& err_msg)
{
  // TODO: go on and type check other stuff instead of throwing at first error
  throw std::runtime_error(
    std::format("[line {}:{}] Err: {}\n", pos.line, pos.column, err_msg));
}

void SemanticAnalyzer::type_check(const parser::ast::Expression& exp)
{
  auto t = exp.accept(*this);
  // TODO: do something with t
}

TEntry SemanticAnalyzer::visit_string_exp(const parser::ast::StringExp& exp)
{
  return string_type;
};

TEntry SemanticAnalyzer::visit_assign_exp(const parser::ast::AssignExp& exp)
{
  auto tvar = exp.var->accept(*this);
  auto trhs = exp.exp->accept(*this);

  if(!can_assign(tvar, trhs)) {
    error_at(exp.position, "type mismatch");
  }
  return unit_type;
};

template <typename T>
bool SemanticAnalyzer::is_type(const types::Type& t)
{
  return typeid(t) == typeid(T);
}

bool SemanticAnalyzer::can_assign(const TEntry& tlhs, const TEntry& trhs)
{
  if(is_type<types::Record>(*tlhs) && is_type<types::Nil>(*trhs)) {
    return true;
  }
  return same_types(tlhs, trhs);
}

TEntry SemanticAnalyzer::skip_name_types(TEntry t)
{
  // the exit guarantee follows in case there are no cycles
  while(is_type<types::Name>(*t)) {
    t = tenv.lookup(dynamic_cast<types::Name*>(t.get())->name).value();
  }
  return t;
}

TEntry SemanticAnalyzer::visit_op_exp(const parser::ast::OpExp& exp)
{
  auto tl = exp.left->accept(*this);
  auto tr = exp.right->accept(*this);

  switch(exp.op) {
  case parser::ast::Operator::plus:
  case parser::ast::Operator::minus:
  case parser::ast::Operator::times:
  case parser::ast::Operator::divide: {

    if(same_types(tl, tr) && is_type<types::Integer>(*tl)) {
      return int_type;
    }
    error_at(exp.position, "operands must be integers");
    break;
  }
  case parser::ast::Operator::equal:
  case parser::ast::Operator::not_equal: {
    // these can be applied to integers, strings, records and arrays

    if(!same_types(tl, tr)) {
      if(is_type<types::Nil>(*tl) && is_type<types::Record>(*tr)) {
        return int_type;
      }
      else if(is_type<types::Nil>(*tr) && is_type<types::Record>(*tl)) {
        return int_type;
      }
      error_at(exp.position, "operands must be of the same type");
    }
    else if(is_type<types::Integer>(*tl) || is_type<types::String>(*tl) ||
            is_type<types::Array>(*tl) || is_type<types::Record>(*tl)) {
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

    if(!same_types(tl, tr)) {
      error_at(exp.position, "operands must be of the same type");
      break;
    }
    else if(is_type<types::Integer>(*tl) || is_type<types::String>(*tl)) {
      return int_type;
    }
    error_at(exp.position, "operands must be integers or strings");
    break;
  }
  default:
    assert(false);
  }
  return nullptr;
};

TEntry SemanticAnalyzer::visit_int_exp(const parser::ast::IntExp& exp)
{
  return int_type;
};

TEntry SemanticAnalyzer::visit_var_exp(const parser::ast::VarExp& exp)
{
  return exp.var->accept(*this);
};

TEntry SemanticAnalyzer::visit_seq_exp(const parser::ast::SeqExp& exp)
{
  TEntry tres = unit_type;
  for(auto& [e, pos] : exp.exps) {
    tres = e->accept(*this);
  }
  return tres;
};

TEntry SemanticAnalyzer::visit_array_exp(const parser::ast::ArrayExp& exp)
{
  auto tsize = exp.size->accept(*this);
  auto tinit = exp.init->accept(*this);

  auto texpr = tenv.lookup(exp.type);
  if(!texpr || !is_type<types::Array>(*texpr.value())) {
    error_at(exp.position,
             std::format("undefined array type '{}'", exp.type.name()));
  }
  else if(!is_type<types::Integer>(*tsize)) {
    error_at(exp.position, "the size of the array must be an integer");
  }
  else {
    auto array_type = dynamic_cast<types::Array*>(texpr.value().get());
    if(!same_types(skip_name_types(array_type->type), tinit)) {
      error_at(exp.position, "the type of the array elements must match");
    }
  }
  return texpr.value();
};

TEntry SemanticAnalyzer::visit_nil_exp(const parser::ast::NilExp& exp)
{
  return nil_type;
};

TEntry SemanticAnalyzer::visit_record_exp(const parser::ast::RecordExp& exp)
{
  auto rtype = tenv.lookup(exp.type);
  if(!rtype || !is_type<types::Record>(*rtype.value())) {
    error_at(exp.position,
             std::format("undefined record type '{}'", exp.type.name()));
  }
  auto record_type = dynamic_cast<types::Record*>(rtype.value().get());

  auto rsize = record_type->fields.size();
  auto esize = exp.fields.size();

  if(rsize != esize) {
    error_at(exp.position,
             std::format("expected {} fields, got {}", rsize, esize));
  }

  for(int i = 0; i < rsize; i++) {
    auto& formal_rfield = record_type->fields[i];
    auto& actual_rfield = exp.fields[i];

    // check field name agreement
    if(formal_rfield.first != actual_rfield.name) {
      error_at(actual_rfield.position,
               std::format("expected field '{}', got '{}'",
                           formal_rfield.first.name(),
                           actual_rfield.name.name()));
    }

    // note that record_type->fields[i] could be a name type
    // this can occur while type checking mutually recursive types
    auto actual_type_rfield = actual_rfield.exp->accept(*this);
    auto actual_formal_rfield = skip_name_types(formal_rfield.second);
    if(!can_assign(actual_formal_rfield, actual_type_rfield)) {
      error_at(actual_rfield.position,
               std::format("expected type '{}' for field '{}', got '{}'",
                           to_string(formal_rfield.second),
                           actual_rfield.name.name(),
                           to_string(actual_type_rfield)));
    }
  }
  return rtype.value();
};

TEntry SemanticAnalyzer::visit_if_exp(const parser::ast::IfExp& exp)
{
  auto tc = exp.cond->accept(*this);
  if(!is_type<types::Integer>(*tc)) {
    error_at(exp.position, "the condition must be an integer");
  }

  auto tt = exp.then->accept(*this);

  if(exp.else_) {
    auto te = exp.else_->accept(*this);
    if(!same_types(tt, te)) {
      error_at(exp.position, "types of then and else branches must match");
    }
    return tt;
  }
  else {
    if(!is_type<types::Unit>(*tt)) {
      error_at(exp.position, "the then branch must not produce any value");
    }
  }
  return unit_type;
};

TEntry SemanticAnalyzer::visit_break_exp(const parser::ast::BreakExp& exp)
{
  if(!can_break) {
    error_at(exp.position, "break statement not within a loop");
  }
  return unit_type;
};

TEntry SemanticAnalyzer::visit_while_exp(const parser::ast::WhileExp& exp)
{
  // condition must be an integer
  if(!is_type<types::Integer>(*exp.cond->accept(*this))) {

    error_at(exp.position, "the condition must be an integer");
  } // body must not produce any value
  else {
    bool can_break_saved = can_break;
    can_break = true;
    if(!is_type<types::Unit>(*exp.body->accept(*this))) {
      error_at(exp.position,
               "the body of the while loop must not produce any value");
    }
    can_break = can_break_saved;
  }
  return unit_type;
};

TEntry SemanticAnalyzer::visit_for_exp(const parser::ast::ForExp& exp)
{
  // high and low must be integers
  if(!is_type<types::Integer>(*exp.low->accept(*this))) {
    error_at(exp.position, "the lower bound must be an integer");
  }
  else if(!is_type<types::Integer>(*exp.high->accept(*this))) {
    error_at(exp.position, "the upper bound must be an integer");
  } // body must not produce any value
  else {
    bool can_break_saved = can_break;
    can_break = true;
    venv.begin_scope();
    venv.enter(exp.var, VarEntry(int_type));
    auto tb = exp.body->accept(*this);
    venv.end_scope();
    can_break = can_break_saved;

    if(!is_type<types::Unit>(*tb)) {
      error_at(exp.position,
               "the body of the for loop must not produce any value");
    }
  }
  return unit_type;
};

TEntry SemanticAnalyzer::visit_call_exp(const parser::ast::CallExp& exp)
{
  auto opt_fentry = venv.lookup(exp.name);
  if(!opt_fentry || !std::holds_alternative<FuncEntry>(opt_fentry.value())) {
    error_at(exp.position,
             std::format("undefined function '{}'", exp.name.name()));
  }
  auto& fentry = std::get<FuncEntry>(opt_fentry.value());

  // check the arguments
  auto fsize = fentry.formals.size();
  auto asize = exp.args.size();

  if(asize != fsize) {
    error_at(exp.position,
             std::format("expected {} arguments, got {}", fsize, asize));
  }

  for(int i = 0; i < asize; i++) {
    auto tactual = exp.args[i]->accept(*this);
    auto expected = fentry.formals[i];
    if(!same_types(skip_name_types(expected), tactual)) {
      error_at(exp.position,
               std::format("argument {} expects type '{}', got '{}'",
                           i,
                           expected->to_string(),
                           tactual->to_string()));
    }
  }
  return skip_name_types(fentry.result);
};

TEntry SemanticAnalyzer::visit_let_exp(const parser::ast::LetExp& exp)
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

void SemanticAnalyzer::visit_func_decl(const parser::ast::FuncDecl& decl)
{
  /*
    To handle mutually recursive functions:

    function is_even(n: int) = if n = 0 then 1 else is_odd(n-1)
    function is_odd(n: int) = if n = 0 then 0 else is_even(n-1) 

    We first augment the venv with the function headers:

    is_even -> FuncEntry(formals=[int], result=int)
    is_odd  -> FuncEntry(formals=[int], result=int)

    So that processing of each body can proceed without undefined references.

  */

  for(auto& fdecl : decl.decls) {

    if(venv.lookup(fdecl->name)) {
      error_at(fdecl->position,
               std::format("redeclaration of function '{}' in the same batch "
                           "of mutually recursive functions",
                           fdecl->name.name()));
    }

    // type check the parameters
    std::vector<TEntry> formals;
    for(auto& param : fdecl->params) {
      auto tparam = tenv.lookup(param.type);
      if(!tparam) {
        error_at(
          param.position,
          std::format("undefined parameter type '{}'", param.type.name()));
      }
      formals.push_back(tparam.value());
    }

    // typecheck return type (not against expression)
    TEntry tresult = unit_type;
    if(fdecl->result) {
      auto fdecl_result = fdecl->result.value();
      auto opt_tresult = tenv.lookup(fdecl_result.first);
      if(!opt_tresult) {
        error_at(
          fdecl_result.second,
          std::format("undefined return type '{}'", fdecl_result.first.name()));
      }
      tresult = opt_tresult.value();
    }

    // add the function header
    venv.enter(fdecl->name, FuncEntry(formals, tresult));
  }

  // go through the bodies
  for(auto& fdecl : decl.decls) {

    venv.begin_scope(); // body scope augmented with formals

    // add formals
    for(auto& param : fdecl->params) {
      venv.enter(param.name, VarEntry(tenv.lookup(param.type).value()));
    }

    // type check return type
    auto func_entry = std::get<FuncEntry>(venv.lookup(fdecl->name).value());
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

void SemanticAnalyzer::visit_var_decl(const parser::ast::VarDecl& decl)
{
  auto tinit = decl.init->accept(*this);
  if(decl.type) {
    auto decl_type = decl.type.value();
    auto tdecl = tenv.lookup(decl_type.first);
    if(!tdecl) {
      error_at(decl_type.second,
               std::format("undefined type '{}'", decl_type.first.name()));
    }
    if(!can_assign(skip_name_types(tdecl.value()), tinit)) {
      error_at(decl_type.second, "type mismatch");
    }
    venv.enter(decl.name, VarEntry(tdecl.value()));
  }
  else {
    if(is_type<types::Nil>(*tinit)) {
      // Nil must be constrained by a record type
      error_at(decl.position, "nil must be constrained by a record type");
    }
    venv.enter(decl.name, VarEntry(tinit));
  }
};                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    

void SemanticAnalyzer::visit_type_decl(const parser::ast::TypeDecl& decl)
{
  // add the headers to the type environment
  for(auto& tdecl : decl.decls) {
    // we register the symbol as a name type, to be resolved in a later pass
    // this way it exists in the environment
    if(tenv.lookup(tdecl->name)) {
      error_at(tdecl->position,
               std::format("redeclaration of type '{}'", tdecl->name.name()));
    }
    tenv.enter(tdecl->name,
               std::make_shared<types::Name>(tdecl->name, nullptr));
  }

  // next we replace all those fake names with the true type
  // however we still need to detect cycles
  for(auto& tdecl : decl.decls) {
    auto actual = tdecl->type->accept(*this);
    tenv.replace(tdecl->name, actual);
  }

  // check for cycles now
  detect_cycles(decl);
}

void SemanticAnalyzer::detect_cycles(const parser::ast::TypeDecl& decl)
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

  std::unordered_set<TEntry> visited;
  for(auto& tdecl : decl.decls) {
    visited.clear();
    auto actual = tenv.lookup(tdecl->name).value();

    // chase the sequence
    while(true) {

      if(visited.contains(actual)) {
        error_at(tdecl->position, "cycle");
      }
      else {
        visited.insert(actual);
      }

      if(is_type<types::Name>(*actual)) {
        actual = tenv.lookup((dynamic_cast<types::Name*>(actual.get()))->name).value();
      }
      else if(is_type<types::Array>(*actual)) {
        actual = dynamic_cast<types::Array*>(actual.get())->type;
      }
      else {
        break;
      }
    }
  }
}

TEntry SemanticAnalyzer::visit_name_type(const parser::ast::NameType& type)
{
  auto t = tenv.lookup(type.name);
  if(!t) {
    error_at(type.position,
             std::format("undefined type '{}'", type.name.name()));
  }
  return t.value();
};

TEntry SemanticAnalyzer::visit_array_type(const parser::ast::ArrayType& type)
{
  auto elem_type = tenv.lookup(type.name);
  if(!elem_type) {
    error_at(type.position,
             std::format("undefined type '{}'", type.name.name()));
  }

  return std::make_shared<types::Array>(elem_type.value());
};

TEntry SemanticAnalyzer::visit_record_type(const parser::ast::RecordType& type)
{
  std::vector<std::pair<symbol::Symbol, TEntry>> fields;
  for(auto& field : type.fields) {
    auto tfield = tenv.lookup(field.type);
    if(!tfield) {
      error_at(field.position,
               std::format("undefined type '{}'", field.type.name()));
    }
    fields.push_back({field.name, tfield.value()});
  }
  return std::make_shared<types::Record>(fields);
};

TEntry SemanticAnalyzer::visit_simple_var(const parser::ast::SimpleVar& var)
{
  auto v = venv.lookup(var.name);
  if(!v || !std::holds_alternative<VarEntry>(v.value())) {
    error_at(var.position,
             std::format("undefined variable '{}'", var.name.name()));
  }
  return skip_name_types(std::get<VarEntry>(v.value()).type);
};

TEntry SemanticAnalyzer::visit_field_var(const parser::ast::FieldVar& var)
{
  auto tlhs = var.var->accept(*this);
  if(!is_type<types::Record>(*tlhs)) {
    error_at(var.position, "operator '.' applies to record types only");
  }
  auto rlhs = dynamic_cast<types::Record*>(tlhs.get());

  // check whether the field name belongs to the record fields
  auto iter =
    std::find_if(rlhs->fields.begin(),
                 rlhs->fields.end(),
                 [&var](const auto& p) { return std::get<0>(p) == var.name; });

  if(iter == rlhs->fields.end()) {
    error_at(var.position,
             std::format("unexpected record field name '{}'", var.name.name()));
  }
  return skip_name_types(std::get<1>(*iter));
};

TEntry SemanticAnalyzer::visit_subscript_var(const parser::ast::SubscriptVar& var)
{
  // [] applicable to arrays only
  auto tlhs = var.var->accept(*this);
  if(!is_type<types::Array>(*tlhs)) {
    error_at(var.position, "operator '[]' applies to array types only");
  }
  auto alhs = dynamic_cast<types::Array*>(tlhs.get());

  // expression must be an integer
  auto texp = var.exp->accept(*this);
  if(!is_type<types::Integer>(*texp)) {
    error_at(var.position, "expression between '[]' must be an integer");
  }

  // the type of the expression is the type of each array element
  return skip_name_types(alhs->type);
};

} // namespace semantic