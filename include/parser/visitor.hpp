#pragma once

namespace parser::ast
{
class SimpleVar;
class FieldVar;
class StringExp;
class AssignExp;
class OpExp;
class IntExp;
class VarExp;
class SeqExp;
class SubscriptVar;
class ArrayExp;
class NilExp;
class RecordExp;
class IfExp;
class BreakExp;
class WhileExp;
class ForExp;
class CallExp;
}; // namespace parser::ast

template <typename T>
class Visitor {
  public:
  T virtual visit_simple_var(
    const std::shared_ptr<const parser::ast::SimpleVar>& var) = 0;
  T virtual visit_string_exp(
    const std::shared_ptr<const parser::ast::StringExp>& exp) = 0;
  T virtual visit_assign_exp(
    const std::shared_ptr<const parser::ast::AssignExp>& exp) = 0;
  T virtual visit_op_exp(
    const std::shared_ptr<const parser::ast::OpExp>& exp) = 0;
  T virtual visit_int_exp(
    const std::shared_ptr<const parser::ast::IntExp>& exp) = 0;
  T virtual visit_var_exp(
    const std::shared_ptr<const parser::ast::VarExp>& exp) = 0;
  T virtual visit_seq_exp(
    const std::shared_ptr<const parser::ast::SeqExp>& exp) = 0;
  T virtual visit_field_var(
    const std::shared_ptr<const parser::ast::FieldVar>& var) = 0;
  T virtual visit_subscript_var(
    const std::shared_ptr<const parser::ast::SubscriptVar>& var) = 0;
  T virtual visit_array_exp(
    const std::shared_ptr<const parser::ast::ArrayExp>& exp) = 0;
  T virtual visit_nil_exp(
    const std::shared_ptr<const parser::ast::NilExp>& exp) = 0;
  T virtual visit_record_exp(
    const std::shared_ptr<const parser::ast::RecordExp>& exp) = 0;
  T virtual visit_if_exp(
    const std::shared_ptr<const parser::ast::IfExp>& exp) = 0;
  T virtual visit_break_exp(
    const std::shared_ptr<const parser::ast::BreakExp>& exp) = 0;
  T virtual visit_while_exp(
    const std::shared_ptr<const parser::ast::WhileExp>& exp) = 0;
  T virtual visit_for_exp(
    const std::shared_ptr<const parser::ast::ForExp>& exp) = 0;
  T virtual visit_call_exp(
    const std::shared_ptr<const parser::ast::CallExp>& exp) = 0;
};