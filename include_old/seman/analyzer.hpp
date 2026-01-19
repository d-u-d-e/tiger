#pragma once
#include <filesystem>
#include <ir/level.hpp>
#include <ir/temp.hpp>
#include <ir/translator.hpp>
#include <ir/tree.hpp>
#include <lexer/position.hpp>
#include <memory>
#include <parser/ast.hpp>
#include <seman/env.hpp>
#include <seman/types.hpp>
#include <seman/visitor.hpp>
#include <stdexcept>
#include <string>
#include <symbol.hpp>

namespace seman
{

using namespace types;

class Exception : public std::runtime_error {
  public:
  Exception(const std::string& what)
    : std::runtime_error(what)
  { }
};

class Analyzer : public TypeCheckerExprVisitor,
                 public TypeCheckerDeclVisitor,
                 public TypeCheckerVarVisitor,
                 public TypeCheckerTypeVisitor

{

  public:
  Analyzer(const std::filesystem::path& filename,
           symbol::StringTable& string_table,
           ir::Translator& translator);
  ir::Exp type_check(const parser::ast::Expression& exp);

  Result visit_string_exp(const parser::ast::StringExp& exp) override;
  Result visit_assign_exp(const parser::ast::AssignExp& exp) override;
  Result visit_op_exp(const parser::ast::OpExp& exp) override;
  Result visit_int_exp(const parser::ast::IntExp& exp) override;
  Result visit_var_exp(const parser::ast::VarExp& exp) override;
  Result visit_seq_exp(const parser::ast::SeqExp& exp) override;
  Result visit_array_exp(const parser::ast::ArrayExp& exp) override;
  Result visit_nil_exp(const parser::ast::NilExp& exp) override;
  Result visit_record_exp(const parser::ast::RecordExp& exp) override;
  Result visit_if_exp(const parser::ast::IfExp& exp) override;
  Result visit_break_exp(const parser::ast::BreakExp& exp) override;
  Result visit_while_exp(const parser::ast::WhileExp& exp) override;
  Result visit_for_exp(const parser::ast::ForExp& exp) override;
  Result visit_call_exp(const parser::ast::CallExp& exp) override;
  Result visit_let_exp(const parser::ast::LetExp& exp) override;

  Result visit_func_decl(const parser::ast::FuncDecl& decl) override;
  Result visit_var_decl(const parser::ast::VarDecl& decl) override;
  Result visit_type_decl(const parser::ast::TypeDecl& decl) override;

  SharedType visit_name_type(const parser::ast::NameType& type) override;
  SharedType visit_array_type(const parser::ast::ArrayType& type) override;
  SharedType visit_record_type(const parser::ast::RecordType& type) override;

  Result visit_simple_var(const parser::ast::SimpleVar& var) override;
  Result visit_field_var(const parser::ast::FieldVar& var) override;
  Result visit_subscript_var(const parser::ast::SubscriptVar& var) override;

  private:
  void add_predefined_types();
  void add_predefined_functions();
  template <typename... Args>
  void add_predef_func(const symbol::Symbol& s, const SharedType& ret, Args&&... formals);

  template <typename T>
  bool is_type(const SharedType& t);
  bool can_assign(const SharedType& tlhs, const SharedType& trhs);
  bool same_types(const SharedType& t1, const SharedType& t2)
  {
    return t1 == t2;
  }
  void error_at(const lexer::Position& pos, const std::string& err_msg);

  struct CurrentLoop {
    std::optional<ir::TempGen::Label>
      lbreak{}; // where we should jump to when we break inside a loop
    ir::Level* level{}; // the level of the function where the loop resides
  };

  void detect_cycles(const parser::ast::TypeDecl& decl);
  SharedType skip_name_types(const SharedType& t);
  CurrentLoop current_loop{};
  symbol::StringTable& string_table;
  ir::Translator& translator;
  std::shared_ptr<ir::Level> current_level{};
  env::Environment<env::TEntry> tenv;
  env::Environment<env::VEntry> venv;
  std::string filename; // for error reporting only
};
} // namespace seman