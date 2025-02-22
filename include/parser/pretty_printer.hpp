#pragma once
#include <parser/ast.hpp>

class ASTVisitor : public Visitor<std::string> {

  public:
  std::string visit_simple_var(
    const std::shared_ptr<const parser::ast::SimpleVar>& var) override
  {
    return indent() + var->field + "SimpleVar(symbol\"" + var->name.name + "\", " + std::to_string(var->position) + ")";
  }

  std::string visit_string_exp(
    const std::shared_ptr<const parser::ast::StringExp>& exp) override
  {
    return indent() + exp->field + "StringExp(\"" + exp->value + "\", " + std::to_string(exp->position) + ")";
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