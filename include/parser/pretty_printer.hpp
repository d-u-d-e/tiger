#pragma once
#include <parser/ast.hpp>

class ASTVisitor : public Visitor<std::string> {

  public:
  std::string visit_simple_var(
    const std::shared_ptr<const parser::ast::SimpleVar>& var) override
  {
    return indent() + var->field + "SimpleVar()";
  }
  std::string visit_field_var(
    const std::shared_ptr<const parser::ast::FieldVar>& var) override
  {
    return indent() + var->field + "FieldVar()";
  }

  std::string visit_string_exp(
    const std::shared_ptr<const parser::ast::StringExp>& exp) override
  {
    return indent() + exp->field + "StringExp()";
  }

  std::string visit_assign_exp(
    const std::shared_ptr<const parser::ast::AssignExp>& exp) override
  {
    std::string result = indent() + exp->field + "AssignExp(\n";
    depth++;
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
    result += exp->left->accept(*this) + ",\n";
    result += indent() + "oper=" +parser::ast::to_string(exp->op) + ",\n";
    result += exp->right->accept(*this) + ",\n";
    result += (indent() + "pos=") + std::to_string(exp->position) + "\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  std::string
  visit_int_exp(const std::shared_ptr<const parser::ast::IntExp>& exp) override
  {
    return indent() + exp->field + "IntExp()";
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
      for(int i = 0; i < exp->exps.size() - 1; i++) {
        result += exp->exps[i]->accept(*this) + ",\n";
      }
      result += exp->exps[size - 1]->accept(*this) + "\n";
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