#include <iostream>
#include <semantic/type_checker.hpp>

namespace semantic
{

static auto int_type = std::make_shared<types::Integer>();
static auto string_type = std::make_shared<types::String>();
static auto nil_type = std::make_shared<types::Nil>();
static auto unit_type = std::make_shared<types::Unit>();

TypeChecker::TypeChecker(symbol::StringTable& string_table)
  : string_table(string_table)
{
  tenv.enter(string_table.symbol("int"), int_type);
  tenv.enter(string_table.symbol("string"), string_type);
  tenv.enter(string_table.symbol("nil"), nil_type);
}

void TypeChecker::error_at(const lexer::Position& pos,
                           const std::string& err_msg)
{
  throw std::runtime_error(
    std::format("[line {}:{}] Err: {}\n", pos.line, pos.column, err_msg));
}

void TypeChecker::check(const parser::ast::Expression& exp)
{
  auto t = exp.accept(*this);
  // TODO: do something with t
}

TEntry TypeChecker::visit_string_exp(const parser::ast::StringExp& exp)
{
  std::cout << "type checking string exp" << std::endl;
  return string_type;
};

TEntry TypeChecker::visit_assign_exp(const parser::ast::AssignExp& exp)
{
  auto tvar = exp.var->accept(*this);
  auto trhs = exp.exp->accept(*this);

  if(check_type<types::Record>(*tvar) && check_type<types::Nil>(*trhs)) {
    // can assign nil to a record variable
    return unit_type;
  }
  else if(!is_same_type(tvar, trhs)) {
    error_at(exp.position, "types do not match");
  }
  return unit_type;
};

template <typename T>
bool TypeChecker::check_type(const types::Type& t)
{
  return typeid(t) == typeid(T);
}

TEntry TypeChecker::visit_op_exp(const parser::ast::OpExp& exp)
{
  std::cout << "type checking op exp" << std::endl;

  auto tl = exp.left->accept(*this);
  auto tr = exp.right->accept(*this);

  switch(exp.op) {
  case parser::ast::Operator::plus:
  case parser::ast::Operator::minus:
  case parser::ast::Operator::times:
  case parser::ast::Operator::divide: {

    if(is_same_type(tl, tr) && check_type<types::Integer>(*tl)) {
      return int_type;
    }
    error_at(exp.position, "operands must be integers");
    break;
  }
  case parser::ast::Operator::equal:
  case parser::ast::Operator::not_equal: {
    // these can be applied to integers, strings, records and arrays

    if(!is_same_type(tl, tr)) {
      if(check_type<types::Nil>(*tl) && check_type<types::Record>(*tr)) {
        return int_type;
      }
      else if(check_type<types::Nil>(*tr) && check_type<types::Record>(*tl)) {
        return int_type;
      }
      error_at(exp.position, "operands must be of the same type");
    }
    else if(check_type<types::Integer>(*tl) || check_type<types::String>(*tl) ||
            check_type<types::Array>(*tl)) {
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

    if(!is_same_type(tl, tr)) {
      error_at(exp.position, "operands must be of the same type");
      break;
    }
    else if(check_type<types::Integer>(*tl) || check_type<types::String>(*tl)) {
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

TEntry TypeChecker::visit_int_exp(const parser::ast::IntExp& exp)
{
  std::cout << "type checking int exp" << std::endl;
  return int_type;
};

TEntry TypeChecker::visit_var_exp(const parser::ast::VarExp& exp)
{
  std::cout << "type checking var exp" << std::endl;
  return exp.var->accept(*this);
};

TEntry TypeChecker::visit_seq_exp(const parser::ast::SeqExp& exp)
{
  TEntry tres = unit_type;
  for(auto& [e, pos] : exp.exps) {
    tres = e->accept(*this);
  }
  return tres;
};

TEntry TypeChecker::visit_array_exp(const parser::ast::ArrayExp& exp)
{
  // TODO
  std::cout << "type checking array exp" << std::endl;
  return nullptr;
};

TEntry TypeChecker::visit_nil_exp(const parser::ast::NilExp& exp)
{
  std::cout << "type checking nil exp" << std::endl;
  return nil_type;
};

TEntry TypeChecker::visit_record_exp(const parser::ast::RecordExp& exp)
{
  //TODO
  std::cout << "type checking record exp" << std::endl;
  return nullptr;
};

TEntry TypeChecker::visit_if_exp(const parser::ast::IfExp& exp)
{
  //TODO
  std::cout << "type checking if exp" << std::endl;
  return nullptr;
};

TEntry TypeChecker::visit_break_exp(const parser::ast::BreakExp& exp)
{
  // TODO: must be inside a for or loop expression
  std::cout << "type checking break exp" << std::endl;
  return unit_type;
};

TEntry TypeChecker::visit_while_exp(const parser::ast::WhileExp& exp)
{
  std::cout << "type checking while exp" << std::endl;

  // condition must be an integer
  if(!check_type<types::Integer>(*exp.cond->accept(*this))) {

    error_at(exp.position, "the condition must be an integer");
  } // body must not produce any value
  else if(!check_type<types::Unit>(*exp.body->accept(*this))) {
    error_at(exp.position,
             "the body of the while loop must not produce any value");
  }
  return unit_type;
};

TEntry TypeChecker::visit_for_exp(const parser::ast::ForExp& exp)
{
  std::cout << "type checking for exp" << std::endl;

  // high and low must be integers
  if(!check_type<types::Integer>(*exp.low->accept(*this))) {
    error_at(exp.position, "the lower bound must be an integer");
  }
  else if(!check_type<types::Integer>(*exp.high->accept(*this))) {
    error_at(exp.position, "the upper bound must be an integer");
  } // body must not produce any value
  else {
    venv.begin_scope();
    venv.enter(exp.var, VarEntry(int_type));
    auto tb = exp.body->accept(*this);
    venv.end_scope();

    if(!check_type<types::Unit>(*tb)) {
      error_at(exp.position,
               "the body of the for loop must not produce any value");
    }
  }
  return unit_type;
};

TEntry TypeChecker::visit_call_exp(const parser::ast::CallExp& exp)
{
  // TODO
  std::cout << "type checking call exp" << std::endl;
  return nullptr;
};

TEntry TypeChecker::visit_let_exp(const parser::ast::LetExp& exp)
{
  std::cout << "type checking let exp" << std::endl;
  TEntry tres{};
  tenv.begin_scope();
  venv.begin_scope();

  for(auto& decl : exp.decls) {
    decl->accept(*this);
  }

  tres = exp.body->accept(*this);
  tenv.end_scope();
  venv.end_scope();
  return tres;
};

void TypeChecker::visit_func_decl(const parser::ast::FuncDecl& decl)
{
  std::cout << "type checking func decl" << std::endl;
  // TODO: mutually recursive functions
  auto& fdecl = decl.decls[0];

  // type check the parameters
  std::vector<TEntry> formals;
  venv.begin_scope(); // variables in the body scope
  for(auto& param : fdecl->params) {
    auto tparam = tenv.lookup(param.type);
    if(!tparam) {
      error_at(param.position,
               std::format("undefined parameter type '{}'", param.type.name()));
    }
    formals.push_back(tparam.value());
    venv.enter(param.name, VarEntry(tparam.value()));
  }

  // type check the return type
  TEntry tresult{};
  if(fdecl->result) {
    auto opt_tresult = tenv.lookup(fdecl->result.value());
    if(!opt_tresult) {
      error_at(fdecl->position,
               std::format("undefined return type '{}'",
                           fdecl->result.value().name()));
    }
    else {
      tresult = opt_tresult.value();
    }
  }

  // add the function to the body scope allowing recursive functions
  venv.enter(fdecl->name, FuncEntry(formals, tresult));

  // type check the body
  auto tbody = fdecl->body->accept(*this);
  if(!is_same_type(tresult, tbody)) {
    // TODO: augment fdecl to hold the return type position
    error_at(fdecl->position, "return type does not match the body type");
  }
  venv.end_scope(); // end body scope

  // add the function to the value env
  venv.enter(fdecl->name, FuncEntry(formals, tresult));
};

void TypeChecker::visit_var_decl(const parser::ast::VarDecl& decl)
{
  std::cout << "type checking var decl" << std::endl;
  auto tvar = decl.init->accept(*this);
  venv.enter(decl.name, VarEntry(tvar));
};

void TypeChecker::visit_type_decl(const parser::ast::TypeDecl& decl)
{
  // TODO
  std::cout << "type checking type decl" << std::endl;
};

TEntry TypeChecker::visit_name_type(const parser::ast::NameType& type)
{
  // TODO
  std::cout << "type checking name type" << std::endl;
  return nullptr;
};

TEntry TypeChecker::visit_array_type(const parser::ast::ArrayType& type)
{
  //TODO
  std::cout << "type checking array type" << std::endl;
  return nullptr;
};

TEntry TypeChecker::visit_record_type(const parser::ast::RecordType& type)
{
  //TODO
  std::cout << "type checking record type" << std::endl;
  return nullptr;
};

TEntry TypeChecker::visit_simple_var(const parser::ast::SimpleVar& var)
{
  std::cout << std::format("type checking simple variable '{}'",
                           var.name.name())
            << std::endl;
  auto v = venv.lookup(var.name);
  if(!v || !std::holds_alternative<VarEntry>(v.value())) {
    error_at(var.position,
             std::format("undefined variable '{}'", var.name.name()));
  }
  // TODO: need to get the actual type by skipping all name types
  return std::get<VarEntry>(v.value()).type;
};

TEntry TypeChecker::visit_field_var(const parser::ast::FieldVar& var)
{
  //TODO
  std::cout << "type checking field var" << std::endl;
  return nullptr;
};

TEntry TypeChecker::visit_subscript_var(const parser::ast::SubscriptVar& var)
{
  //TODO
  std::cout << "type checking subscript var" << std::endl;
  return nullptr;
};

} // namespace semantic