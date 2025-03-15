#include <parser/ast.hpp>
#include <seman/escape.hpp>

namespace seman
{

void EscapeFinder::visit_type_decl(parser::ast::TypeDecl& decl) { }

void EscapeFinder::visit_string_exp(parser::ast::StringExp& exp) { }

void EscapeFinder::visit_int_exp(parser::ast::IntExp& exp) { }

void EscapeFinder::visit_nil_exp(parser::ast::NilExp& exp) { }

void EscapeFinder::visit_break_exp(parser::ast::BreakExp& exp) { }

void EscapeFinder::visit_assign_exp(parser::ast::AssignExp& exp)
{
  exp.var->accept(*this);
  exp.exp->accept(*this);
}

void EscapeFinder::visit_op_exp(parser::ast::OpExp& exp)
{
  exp.left->accept(*this);
  exp.right->accept(*this);
}

void EscapeFinder::visit_var_exp(parser::ast::VarExp& exp)
{
  exp.var->accept(*this);
}

void EscapeFinder::visit_seq_exp(parser::ast::SeqExp& exp)
{
  for(auto& e : exp.exps) {
    e.first->accept(*this);
  }
}

void EscapeFinder::visit_array_exp(parser::ast::ArrayExp& exp)
{
  exp.init->accept(*this);
  exp.size->accept(*this);
}

void EscapeFinder::visit_record_exp(parser::ast::RecordExp& exp)
{
  for(auto& f : exp.fields) {
    f.exp->accept(*this);
  }
}

void EscapeFinder::visit_if_exp(parser::ast::IfExp& exp)
{
  exp.cond->accept(*this);
  exp.then->accept(*this);
  if(exp.else_) {
    exp.else_->accept(*this);
  }
}

void EscapeFinder::visit_while_exp(parser::ast::WhileExp& exp)
{
  exp.cond->accept(*this);
  exp.body->accept(*this);
}

void EscapeFinder::visit_for_exp(parser::ast::ForExp& exp)
{
  exp.low->accept(*this);
  exp.high->accept(*this);
  depth++;
  *exp.escape = false;
  table.enter(exp.var, Escape(depth, exp.escape));
  exp.body->accept(*this);
  depth--;
}

void EscapeFinder::visit_call_exp(parser::ast::CallExp& exp)
{
  for(auto& arg : exp.args) {
    arg->accept(*this);
  }
}

void EscapeFinder::visit_let_exp(parser::ast::LetExp& exp)
{
  depth++;
  for(auto& d : exp.decls) {
    d->accept(*this);
  }
  exp.body->accept(*this);
  depth--;
}

void EscapeFinder::visit_simple_var(parser::ast::SimpleVar& var)
{
  if(auto v = table.lookup(var.name); v && v->depth < depth) {
    *(v->ref) = true;
  }
}

void EscapeFinder::visit_field_var(parser::ast::FieldVar& var)
{
  var.var->accept(*this);
}

void EscapeFinder::visit_subscript_var(parser::ast::SubscriptVar& var)
{
  var.var->accept(*this);
  var.exp->accept(*this);
}

void EscapeFinder::visit_var_decl(parser::ast::VarDecl& decl)
{
  *decl.escape = false;
  table.enter(decl.name, Escape(depth, decl.escape));
  decl.init->accept(*this);
}

void EscapeFinder::visit_func_decl(parser::ast::FuncDecl& decl)
{
  depth++;
  for(auto& d : decl.decls) {
    for(auto& p : d->params) {
      *p.escape = false;
      table.enter(p.name, Escape(depth, p.escape));
    }
    d->body->accept(*this);
  }
  depth--;
}

} // namespace seman