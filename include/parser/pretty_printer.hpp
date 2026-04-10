#pragma once
#include "parser/ast.hpp"
#include "parser/visitor.hpp"

namespace parser::ast
{

class PrettyPrinter : public PrettyPrinterExprVisitor,
                      public PrettyPrinterTypeVisitor,
                      public PrettyPrinterDeclVisitor,
                      public PrettyPrinterVarVisitor
{

  public:
  auto visit_simple_var(const parser::ast::Var& var) -> std::string override
  {

    return std::format(
      "{}{}SimpleVar{{symbol\"{}\", pos={}}}", indent(), var.field, var.name.str(), var.position);
  }

  auto visit_field_var(const parser::ast::FieldVar& var) -> std::string override
  {
    std::string result = indent() + var.field + "FieldVar{\n";
    depth++;
    var.var->field = "var=";
    result += var.var->accept(*this) + ",\n";
    result += indent() + "symbol\"" + var.name.str() + "\", pos=" + var.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_string_exp(const parser::ast::StringExp& exp) -> std::string override
  {
    return std::format(
      "{}{}StringExp{{\"{}\", pos={}}}", indent(), exp.field, exp.value, exp.position);
  }

  auto visit_assign_exp(const parser::ast::AssignExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "AssignExp{\n";
    depth++;
    exp.var->field = "var=";
    exp.exp->field = "exp=";
    result += exp.var->accept(*this) + ",\n";
    result += exp.exp->accept(*this) + ",\n";
    result += (indent() + "pos=") + exp.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_op_exp(const parser::ast::OpExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "OpExp{\n";
    depth++;
    exp.left->field = "left=";
    exp.right->field = "right=";
    result += exp.left->accept(*this) + ",\n";
    result += indent() + "oper=" + parser::ast::to_string(exp.op) + ",\n";
    result += exp.right->accept(*this) + ",\n";
    result += (indent() + "pos=") + exp.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_int_exp(const parser::ast::IntExp& exp) -> std::string override
  {
    return std::format("{}{}IntExp{{{}}}", indent(), exp.field, std::to_string(exp.value));
  }

  auto visit_var_exp(const parser::ast::VarExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "VarExp{\n";
    depth++;
    result += exp.var->accept(*this) + ",\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_seq_exp(const parser::ast::SeqExp& exp) -> std::string override
  {
    auto size = exp.exps.size();
    std::string result = indent() + exp.field + "SeqExp{[\n";

    if(size != 0)
    {
      depth++;
      for(size_t i = 0; i < size; i++)
      {
        const auto& v = exp.exps[i];
        const auto& ve = std::get<0>(v);
        ve->field = "(";
        result += ve->accept(*this) + ", pos=" + std::get<1>(v).to_string() + ")" +
                  ((i == size - 1) ? "\n" : ",\n");
      }
      depth--;
    }
    result += indent() + "]}";
    return result;
  }

  auto visit_subscript_var(const parser::ast::SubscriptVar& var) -> std::string override
  {
    std::string result = indent() + var.field + "SubscriptVar{\n";
    depth++;
    var.var->field = "var=";
    var.exp->field = "exp=";
    result += var.var->accept(*this) + ",\n";
    result += var.exp->accept(*this) + ",\n";
    result += (indent() + "pos=") + var.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_array_exp(const parser::ast::ArrayExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "ArrayExp{\n";
    depth++;
    result += (indent() + "type=") + exp.type.str() + ",\n";
    exp.size->field = "size=";
    result += exp.size->accept(*this) + ",\n";
    exp.init->field = "init=";
    result += exp.init->accept(*this) + ",\n";
    result += (indent() + "pos=") + exp.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_nil_exp(const parser::ast::NilExp& exp) -> std::string override
  {
    return std::format("{}{}NilExp{{{}}}", indent(), exp.field, "nil");
  }

  auto visit_record_exp(const parser::ast::RecordExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "RecordExp{\n";
    depth++;
    result += (indent() + "type=symbol\"") + exp.type.str() + "\",\n";
    result += indent() + "fields=[\n";
    depth++;
    auto size = exp.fields.size();
    for(size_t i = 0; i < size; i++)
    {
      const auto& field = exp.fields[i];
      field.exp->field =
        "(symbol\"" + field.name.str() + "\", pos=" + field.position.to_string() + ", exp=";
      result += field.exp->accept(*this) + ")" + ((i == size - 1) ? "\n" : ",\n");
    }
    depth--;
    result += indent() + "],\n";
    result += (indent() + "pos=") + exp.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_if_exp(const parser::ast::IfExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "IfExp{\n";
    depth++;
    exp.cond->field = "cond=";
    result += exp.cond->accept(*this) + ",\n";
    exp.then->field = "then=";
    result += exp.then->accept(*this) + ",\n";

    if(exp.else_)
    {
      exp.else_->field = "else=";
      result += exp.else_->accept(*this) + ",\n";
    }
    result += (indent() + "pos=") + exp.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_break_exp(const parser::ast::BreakExp& exp) -> std::string override
  {
    return std::format("{}{}BreakExp{{pos={}}}", indent(), exp.field, exp.position);
  }

  auto visit_while_exp(const parser::ast::WhileExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "WhileExp{\n";
    depth++;
    exp.cond->field = "cond=";
    result += exp.cond->accept(*this) + ",\n";
    exp.body->field = "body=";
    result += exp.body->accept(*this) + ",\n";
    result += indent() + "pos=" + exp.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_for_exp(const parser::ast::ForExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "ForExp{\n";
    depth++;
    result += (indent() + "var=symbol\"") + exp.var.str() + "\",\n";
    exp.low->field = "low=";
    result += exp.low->accept(*this) + ",\n";
    exp.high->field = "high=";
    result += exp.high->accept(*this) + ",\n";
    exp.body->field = "body=";
    result += exp.body->accept(*this) + ",\n";
    result += indent() + "pos=" + exp.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_call_exp(const parser::ast::CallExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "CallExp{\n";
    depth++;
    exp.callee->field = "callee=";
    result += exp.callee->accept(*this) + "\n";
    result += indent() + "args=[\n";
    depth++;
    auto size = exp.args.size();
    for(size_t i = 0; i < size; i++)
    {
      const auto& arg = exp.args[i];
      result += arg->accept(*this) + ((i == size - 1) ? "\n" : ",\n");
    }
    depth--;
    result += indent() + "],\n";
    result += (indent() + "pos=") + exp.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_let_exp(const parser::ast::LetExp& exp) -> std::string override
  {
    std::string result = indent() + exp.field + "LetExp{\n";
    depth++;
    result += indent() + "decls=[\n";
    depth++;
    auto size = exp.decls.size();
    for(size_t i = 0; i < size; i++)
    {
      const auto& decl = exp.decls[i];
      result += decl->accept(*this) + ((i == size - 1) ? "\n" : ",\n");
    }
    depth--;
    result += indent() + "],\n";
    exp.body->field = "body=";
    result += exp.body->accept(*this) + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_func_decl(const parser::ast::FuncDecl& decl) -> std::string override
  {
    std::string result = indent() + decl.field + "FuncDecl{\n";
    auto size = decl.decls.size();
    for(size_t i = 0; i < size; i++)
    {
      const auto& fdecl = decl.decls[i];
      result += visit_single_func_decl(*fdecl) + ((i == size - 1) ? "\n" : ",\n");
    }
    result += indent() + "}";
    return result;
  }

  auto visit_var_decl(const parser::ast::VarDecl& decl) -> std::string override
  {
    std::string result = indent() + decl.field + "VarDecl{\n";
    depth++;
    result += indent() + "name=symbol\"" + decl.name.str() + "\",\n";
    if(decl.type)
    {
      result += indent() + "type=symbol\"" + decl.type.value().first.str() + "\",\n";
    }
    decl.init->field = "init=";
    result += decl.init->accept(*this) + ",\n";
    result += (indent() + "pos=") + decl.position.to_string() + "\n";
    depth--;
    return result + indent() + "}";
  }

  auto visit_type_decl(const parser::ast::TypeDecl& decl) -> std::string override
  {
    std::string result = indent() + decl.field + "TypeDecl{\n";
    auto size = decl.decls.size();
    for(size_t i = 0; i < size; i++)
    {
      const auto& tdecl = decl.decls[i];
      result += indent() + "(\n";
      depth++;
      result += indent() + "name=symbol\"" + tdecl->name.str() + "\",\n";
      tdecl->type->field = "ty=";
      result += tdecl->type->accept(*this) + ",\n";
      result += (indent() + "pos=") + tdecl->position.to_string() + "\n";
      depth--;
      result += indent() + ")" + ((i == size - 1) ? "\n" : ",\n");
    }
    result += indent() + "}";
    return result;
  }

  auto visit_name_type(const parser::ast::NameType& type) -> std::string override
  {
    std::string result = indent() + type.field + "NameType{\n";
    depth++;
    result += indent() + "name=symbol\"" + type.name.str() + "\",\n";
    result += (indent() + "pos=") + type.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_array_type(const parser::ast::ArrayType& type) -> std::string override
  {
    std::string result = indent() + type.field + "ArrayType{\n";
    depth++;
    result += indent() + "name=symbol\"" + type.name.str() + "\",\n";
    result += (indent() + "pos=") + type.position.to_string() + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_record_type(const parser::ast::RecordType& type) -> std::string override
  {
    std::string result = indent() + type.field + "RecordType{\n";
    depth++;
    auto size = type.fields.size();
    for(size_t i = 0; i < size; i++)
    {
      const auto& field = type.fields[i];
      result += visit_single_field(field) + ((i == size - 1) ? "\n" : ",\n");
    }
    depth--;
    result += indent() + "}";
    return result;
  }

  auto visit_function_type(const parser::ast::FunctionType& type) -> std::string override
  {
    std::string result = indent() + type.field + "FunctionType{\n";
    depth++;
    auto size = type.arg_types.size();

    if(size == 0)
    {
      result += indent() + "() -> \n";
    }
    else
    {
      result += indent() + "(\n";
      depth++;
      for(size_t i = 0; i < size; i++)
      {
        const auto& arg = type.arg_types[i];
        auto arg_value = arg->accept(*this);
        result += arg_value + ((i == size - 1) ? "\n" : ",\n");
      }
      depth--;
      result += indent() + ") -> \n";
    }

    result += type.ret_type->accept(*this) + "\n";
    depth--;
    result += indent() + "}";
    return result;
  }

  private:
  auto visit_single_field(const parser::ast::Field_& f) -> std::string
  {
    std::string result = indent() + "(symbol\"" + f.name.str() +
                         "\", pos=" + f.position.to_string() + ", type=symbol\"" + f.type.str() +
                         "\")";
    return result;
  }

  std::string visit_single_func_decl(const parser::ast::FuncDecl_& decl)
  {
    std::string result = indent() + "(\n";
    depth++;
    result += indent() + "name=symbol\"" + decl.name.str() + "\",\n";
    result += indent() + "params=[\n";
    depth++;
    auto size = decl.params.size();
    for(size_t i = 0; i < size; i++)
    {
      const auto& param = decl.params[i];
      result += visit_single_field(param) + ((i == size - 1) ? "\n" : ",\n");
    }
    depth--;
    result += indent() + "]\n";

    if(decl.result)
    {
      result += indent() + "result=symbol\"" + decl.result.value().first.str() + "\",\n";
    }
    decl.body->field = "body=";
    result += decl.body->accept(*this) + ",\n";
    result += (indent() + "pos=") + decl.position.to_string() + "\n";
    depth--;
    result += indent() + ")";
    return result;
  }

  static auto indent(size_t depth) -> std::string
  {
    std::string result(depth * 2, ' ');
    return result;
  }
  [[nodiscard]] auto indent() const -> std::string
  {
    return indent(depth);
  }
  size_t depth{0};
};

} // namespace parser::ast