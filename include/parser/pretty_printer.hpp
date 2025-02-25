#pragma once
#include <parser/ast.hpp>

class ASTVisitor : public Visitor<std::string> {

  public:
  std::string visit_simple_var(
    const std::shared_ptr<const parser::ast::SimpleVar>& var) override
  {
    return indent() + var->field + "SimpleVar(symbol\"" + var->name.str +
           "\", " + std::to_string(var->position) + ")";
  }

  std::string visit_field_var(
    const std::shared_ptr<const parser::ast::FieldVar>& var) override
  {
    std::string result = indent() + var->field + "FieldVar(\n";
    depth++;
    result += var->var->accept(*this) + ",\n";
    result += indent() + "symbol\"" + var->name.str + "\", " +
              std::to_string(var->position) + "\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string visit_string_exp(
    const std::shared_ptr<const parser::ast::StringExp>& exp) override
  {
    return indent() + exp->field + "StringExp(\"" + exp->value + "\", " +
           std::to_string(exp->position) + ")";
  }

  std::string visit_assign_exp(
    const std::shared_ptr<const parser::ast::AssignExp>& exp) override
  {
    std::string result = indent() + exp->field + "AssignExp(\n";
    depth++;
    exp->left->field = "var=";
    exp->right->field = "exp=";
    result += exp->left->accept(*this) + ",\n";
    result += exp->right->accept(*this) + ",\n";
    result += (indent() + "pos=") + std::to_string(exp->position) + "\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string
  visit_op_exp(const std::shared_ptr<const parser::ast::OpExp>& exp) override
  {
    std::string result = indent() + exp->field + "OpExp(\n";
    depth++;
    exp->left->field = "left=";
    exp->right->field = "right=";
    result += exp->left->accept(*this) + ",\n";
    result += indent() + "oper=" + parser::ast::to_string(exp->op) + ",\n";
    result += exp->right->accept(*this) + ",\n";
    result += (indent() + "pos=") + std::to_string(exp->position) + "\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string
  visit_int_exp(const std::shared_ptr<const parser::ast::IntExp>& exp) override
  {
    return indent() + exp->field + "IntExp(" + std::to_string(exp->value) + ")";
  }

  std::string
  visit_var_exp(const std::shared_ptr<const parser::ast::VarExp>& exp) override
  {
    std::string result = indent() + exp->field + "VarExp(\n";
    depth++;
    result += exp->var->accept(*this) + ",\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string
  visit_seq_exp(const std::shared_ptr<const parser::ast::SeqExp>& exp) override
  {
    int size = exp->exps.size();
    std::string ind = indent();
    std::string result = ind + exp->field + "SeqExp[\n";

    if(size != 0) {
      depth++;
      for(int i = 0; i < size; i++) {
        auto& v = exp->exps[i];
        auto& ve = std::get<0>(v);
        ve->field = "[";
        result += ve->accept(*this) + ", " + std::to_string(std::get<1>(v)) +
                  "]" + ((i == size - 1) ? "\n" : ",\n");
      }
      depth--;
    }

    result += ind + "]";
    return result;
  }

  std::string visit_subscript_var(
    const std::shared_ptr<const parser::ast::SubscriptVar>& var) override
  {
    std::string result = indent() + var->field + "SubscriptVar(\n";
    depth++;
    var->var->field = "var=";
    var->exp->field = "exp=";
    result += var->var->accept(*this) + ",\n";
    result += var->exp->accept(*this) + ",\n";
    result += (indent() + "pos=") + std::to_string(var->position) + ",\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string visit_array_exp(
    const std::shared_ptr<const parser::ast::ArrayExp>& exp) override
  {
    std::string result = indent() + exp->field + "ArrayExp(\n";
    depth++;
    result += (indent() + "type=") + exp->type.str + ",\n";
    exp->size->field = "size=";
    result += exp->size->accept(*this) + ",\n";
    exp->init->field = "init=";
    result += exp->init->accept(*this) + ",\n";
    result += (indent() + "pos=") + std::to_string(exp->position) + ",\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string
  visit_nil_exp(const std::shared_ptr<const parser::ast::NilExp>& exp) override
  {
    return indent() + exp->field + "NilExp(nil)";
  }

  std::string visit_record_exp(
    const std::shared_ptr<const parser::ast::RecordExp>& exp) override
  {
    std::string result = indent() + exp->field + "RecordExp(\n";
    depth++;
    result += (indent() + "type=symbol\"") + exp->type.str + "\",\n";
    result += indent() + "fields={\n";
    depth++;
    for(int i = 0; i < exp->fields.size(); i++) {
      auto& field = exp->fields[i];
      field.exp->field = "[symbol\"" + field.name.str +
                         "\", pos=" + std::to_string(field.position) + ", exp=";
      result += field.exp->accept(*this) + "],\n";
    }
    depth--;
    result += indent() + "}\n";
    result += (indent() + "pos=") + std::to_string(exp->position) + ",\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string
  visit_if_exp(const std::shared_ptr<const parser::ast::IfExp>& exp) override
  {
    std::string result = indent() + exp->field + "IfExp(\n";
    depth++;
    exp->cond->field = "cond=";
    result += exp->cond->accept(*this) + ",\n";
    exp->then->field = "then=";
    result += exp->then->accept(*this) + ",\n";

    if(exp->else_) {
      exp->else_->field = "else=";
      result += exp->else_->accept(*this) + ",\n";
    }
    result += (indent() + "pos=") + std::to_string(exp->position) + ",\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string visit_break_exp(
    const std::shared_ptr<const parser::ast::BreakExp>& exp) override
  {
    return indent() + exp->field +
           "BreakExp(pos=" + std::to_string(exp->position) + ")";
  }

  std::string visit_while_exp(
    const std::shared_ptr<const parser::ast::WhileExp>& exp) override
  {
    std::string result = indent() + exp->field + "WhileExp(\n";
    depth++;
    exp->cond->field = "cond=";
    result += exp->cond->accept(*this) + ",\n";
    exp->body->field = "body=";
    result += exp->body->accept(*this) + ",\n";
    result += (indent() + "pos=") + std::to_string(exp->position) + ",\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string
  visit_for_exp(const std::shared_ptr<const parser::ast::ForExp>& exp) override
  {
    std::string result = indent() + exp->field + "ForExp(\n";
    depth++;
    result += (indent() + "var=symbol\"") + exp->var.str + "\",\n";
    exp->low->field = "low=";
    result += exp->low->accept(*this) + ",\n";
    exp->high->field = "high=";
    result += exp->high->accept(*this) + ",\n";
    exp->body->field = "body=";
    result += exp->body->accept(*this) + ",\n";
    result += (indent() + "pos=") + std::to_string(exp->position) + ",\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string visit_call_exp(
    const std::shared_ptr<const parser::ast::CallExp>& exp) override
  {
    std::string result = indent() + exp->field + "CallExp(\n";
    depth++;
    result += indent() + "func=symbol\"" + exp->name.str + "\",\n";
    result += indent() + "args=[,\n";
    depth++;
    for(int i = 0; i < exp->args.size(); i++) {
      auto& arg = exp->args[i];
      result += arg->accept(*this) + ",\n";
    }
    depth--;
    result += indent() + "],\n";
    result += (indent() + "pos=") + std::to_string(exp->position) + ",\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string
  visit_let_exp(const std::shared_ptr<const parser::ast::LetExp>& exp) override
  {
    std::string result = indent() + exp->field + "LetExp(\n";
    depth++;
    result += indent() + "decls=[\n";
    depth++;

    for(auto& decl : exp->decls) {
      result += decl->accept(*this) + ",\n";
    }

    depth--;
    result += indent() + "],\n";
    exp->body->field = "body=";
    result += exp->body->accept(*this) + "\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string visit_func_decl(
    const std::shared_ptr<const parser::ast::FuncDecl>& decl) override
  {
    std::string result = indent() + decl->field + "FuncDecl(\n";
    for(auto& fdecl : decl->decls) {
      result += indent() + "{\n";
      depth++;
      result += indent() + "name=symbol\"" + fdecl->name.str + "\",\n";
      result += indent() + "params=[\n";
      depth++;
      for(auto& arg : fdecl->params) {
        result += indent() + "(symbol\"" + arg.name.str +
                  "\", pos=" + std::to_string(arg.position) +
                  ", type=symbol\"" + arg.type.str + "\"),\n";
      }
      depth--;
      result += indent() + "],\n";
      if(fdecl->result) {
        result +=
          indent() + "result=symbol\"" + fdecl->result.value().str + "\",\n";
      }
      fdecl->body->field = "body=";
      result += fdecl->body->accept(*this) + ",\n";
      result += (indent() + "pos=") + std::to_string(fdecl->position) + "\n";
      depth--;
      result += indent() + "},\n";
    }

    result += indent() + ")";
    return result;
  }

  private:
  std::string indent(int depth)
  {
    std::string result(depth * 2, ' ');
    return result;
  }
  std::string indent()
  {
    return indent(depth);
  }
  int depth{0};
};