#pragma once
#include "ir/tree.hpp"
#include "parser/ast.hpp"
#include "semant/types.hpp"
#include "semant/visitor.hpp"
#include "string_table.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>

namespace semant
{
class Exception : public std::runtime_error
{
  public:
  Exception(const std::string& what)
    : std::runtime_error(what)
  { }
};

template <typename Translator>
class Analyzer : TypeCheckerExprVisitor,
                 TypeCheckerDeclVisitor,
                 TypeCheckerVarVisitor,
                 TypeCheckerTypeVisitor
{
  public:
  Analyzer(const std::filesystem::path& filename,
           StringTable& string_table,
           Translator& translator);
  ir::Exp type_check(const parser::ast::Expression& exp);

  private:
  types::Result visit_string_exp(const parser::ast::StringExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_assign_exp(const parser::ast::AssignExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_op_exp(const parser::ast::OpExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_int_exp(const parser::ast::IntExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_var_exp(const parser::ast::VarExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_seq_exp(const parser::ast::SeqExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_array_exp(const parser::ast::ArrayExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_nil_exp(const parser::ast::NilExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_record_exp(const parser::ast::RecordExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_if_exp(const parser::ast::IfExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_break_exp(const parser::ast::BreakExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_while_exp(const parser::ast::WhileExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_for_exp(const parser::ast::ForExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_call_exp(const parser::ast::CallExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_let_exp(const parser::ast::LetExp& exp) override
  {
    static_cast<void>(exp);
    return types::Result{};
  }

  types::Result visit_func_decl(const parser::ast::FuncDecl& decl) override
  {
    static_cast<void>(decl);
    return types::Result{};
  }

  types::Result visit_var_decl(const parser::ast::VarDecl& decl) override
  {
    static_cast<void>(decl);
    return types::Result{};
  }

  types::Result visit_type_decl(const parser::ast::TypeDecl& decl) override
  {
    static_cast<void>(decl);
    return types::Result{};
  }

  types::SharedType visit_name_type(const parser::ast::NameType& type) override
  {
    static_cast<void>(type);
    return types::SharedType{};
  }

  types::SharedType visit_array_type(const parser::ast::ArrayType& type) override
  {
    static_cast<void>(type);
    return types::SharedType{};
  }

  types::SharedType visit_record_type(const parser::ast::RecordType& type) override
  {
    static_cast<void>(type);
    return types::SharedType{};
  }

  types::Result visit_simple_var(const parser::ast::SimpleVar& var) override
  {
    static_cast<void>(var);
    return types::Result{};
  }

  types::Result visit_field_var(const parser::ast::FieldVar& var) override
  {
    static_cast<void>(var);
    return types::Result{};
  }

  types::Result visit_subscript_var(const parser::ast::SubscriptVar& var) override
  {
    static_cast<void>(var);
    return types::Result{};
  }

  private:
  StringTable& string_table;
  Translator& translator;
  std::string filename; // for error reporting only
};

// implementations
template <typename Translator>
Analyzer<Translator>::Analyzer(const std::filesystem::path& filename,
                               StringTable& string_table,
                               Translator& translator)
  : string_table(string_table)
  , translator(translator)
  , filename(filename)
{
  // TODO
}

template <typename Translator>
ir::Exp Analyzer<Translator>::type_check(const parser::ast::Expression& exp)
{
  auto t = exp.accept(*this);
  return std::move(t.ir);
}

} // namespace semant