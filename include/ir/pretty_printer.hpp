#pragma once
#include <ir/tree.hpp>
#include <ir/visitor.hpp>

namespace ir
{

class PrettyPrinter : public PrettyPrinterExprVisitor,
                      public PrettyPrinterStmtVisitor {

  std::string visit_const_exp(const ConstExp& exp) override
  {
    return std::format("ConstExp({})", exp.v);
  }

  std::string visit_name_exp(const NameExp& exp) override
  {
    return std::format("NameExp(\"{}\")", exp.label.str());
  }

  std::string visit_temp_exp(const TempExp& exp) override
  {
    return std::format("NameExp(t{})", exp.temp);
  }

  std::string visit_binop_exp(const BinOpExp& exp) override
  {
    // TODO
    return "";
  }
  std::string visit_mem_exp(const MemExp& exp) override
  {
    // TODO
    return "";
  }
  std::string visit_call_exp(const CallExp& exp) override
  {
    // TODO
    return "";
  }
  std::string visit_eseq_exp(const ESeqExp& exp) override
  {
    // TODO
    return "";
  }

  std::string visit_move_stmt(const MoveStmt& stmt) override
  {
    // TODO
    return "";
  }

  std::string visit_exp_stmt(const ExpStmt& stmt) override
  {
    // TODO
    return "";
  }

  std::string visit_jump_stmt(const JumpStmt& stmt) override
  {
    // TODO
    return "";
  }

  std::string visit_cjump_stmt(const CJumpStmt& stmt) override
  {
    // TODO
    return "";
  }

  std::string visit_seq_stmt(const SeqStmt& stmt) override
  {
    // TODO
    return "";
  }

  std::string visit_label_stmt(const LabelStmt& stmt) override
  {
    return std::format("LabelStmt({})", stmt.label.str());
  }
};

} // namespace ir