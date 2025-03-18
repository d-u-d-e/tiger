#pragma once
#include <cassert>
#include <ir/tree.hpp>
#include <ir/visitor.hpp>

namespace ir
{

class PrettyPrinter : public PrettyPrinterExprVisitor,
                      public PrettyPrinterStmtVisitor {

  std::string visit_const_exp(const ConstExp& exp) override
  {
    return std::format("{}ConstExp({})", indent(), exp.v);
  }

  std::string visit_name_exp(const NameExp& exp) override
  {
    return std::format("{}NameExp(\"{}\")", indent(), exp.label.str());
  }

  std::string visit_temp_exp(const TempExp& exp) override
  {
    return std::format("{}NameExp(t{})", indent(), exp.temp);
  }

  std::string visit_binop_exp(const BinOpExp& exp) override
  {
    std::string r = indent() + "BinOpExp(\n";
    depth++;
    r += indent() + name(exp.op) + ",\n";
    r += exp.left->accept(*this) + ",\n";
    r += exp.right->accept(*this) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string visit_mem_exp(const MemExp& exp) override
  {
    std::string r = indent() + "MemExp(\n";
    depth++;
    r += exp.a->accept(*this) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string visit_call_exp(const CallExp& exp) override
  {
    std::string r = indent() + "CallExpr(\n";
    depth++;
    r += exp.fun->accept(*this) + ",\n";
    r += indent() + "[\n";
    auto size = exp.args.size();
    depth++;
    for(int i = 0; i < size; i++) {
      auto& arg = exp.args[i];
      r += arg->accept(*this) + ((i == size - 1) ? "\n" : ",\n");
    }
    depth--;
    r += indent() + "]\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string visit_eseq_exp(const ESeqExp& exp) override
  {
    std::string r = indent() + "ESeq(\n";
    depth++;
    r += exp.stmt->accept(*this) + ",\n";
    r += exp.exp->accept(*this) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string visit_move_stmt(const MoveStmt& stmt) override
  {
    std::string r = indent() + "MoveStmt(\n";
    depth++;
    r += stmt.left->accept(*this) + ",\n";
    r += stmt.right->accept(*this) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string visit_exp_stmt(const ExpStmt& stmt) override
  {
    std::string r = indent() + "ExpStmt(\n";
    depth++;
    r += stmt.exp->accept(*this) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string visit_jump_stmt(const JumpStmt& stmt) override
  {
    std::string r = indent() + "JumpStmt(\n";
    depth++;
    r += stmt.a->accept(*this) + "\n";
    r += indent() + "[";
    depth++;
    auto size = stmt.labels.size();
    for(int i = 0; i < size; i++) {
      auto& l = stmt.labels[i];
      r += l.str() + ((i == size - 1) ? "\n" : ",\n");
    }
    depth--;
    r += indent() + "]";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string visit_cjump_stmt(const CJumpStmt& stmt) override
  {
    std::string r = indent() + "CondJumpStmt(\n";
    depth++;
    r += indent() + name(stmt.op) + ",\n";
    r += indent() + stmt.tlabel.str() + ",\n";
    r += indent() + stmt.flabel.str() + ",\n";
    r += stmt.lexp->accept(*this) + ",\n";
    r += stmt.rexp->accept(*this) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string visit_seq_stmt(const SeqStmt& stmt) override
  {
    std::string r = indent() + "SeqStmt[\n";
    depth++;
    r += stmt.stm1->accept(*this) + ",\n";
    r += stmt.stm2->accept(*this) + "\n";
    depth--;
    r += indent() + "]";
    return r;
  }

  std::string visit_label_stmt(const LabelStmt& stmt) override
  {
    return std::format("{}LabelStmt({})", indent(), stmt.label.str());
  }

  private:
  inline std::string indent(int depth)
  {
    std::string result(depth * 2, ' ');
    return result;
  }
  inline std::string indent()
  {
    return indent(depth);
  }
  int depth{0};

  std::string name(BinaryOp op)
  {
    std::string r = "";
    switch(op) {
    case BinaryOp::and_:
      r = "and";
      break;
    case BinaryOp::or_:
      r = "or";
      break;
    case BinaryOp::xor_:
      r = "xor";
      break;
    case BinaryOp::lshift:
      r = "lshift";
      break;
    case BinaryOp::rshift:
      r = "rshift";
      break;
    case BinaryOp::arshift:
      r = "arshift";
      break;
    case BinaryOp::minus:
      r = "minus";
      break;
    case BinaryOp::plus:
      r = "plus";
      break;
    case BinaryOp::div:
      r = "div";
      break;
    case BinaryOp::mul:
      r = "mul";
      break;
    default:
      assert(false);
    }
    return r;
  }

  std::string name(RelOp op)
  {
    std::string r = "";
    switch(op) {
    case RelOp::eq:
      r = "eq";
      break;
    case RelOp::ne:
      r = "ne";
      break;
    case RelOp::ge:
      r = "ge";
      break;
    case RelOp::le:
      r = "le";
      break;
    case RelOp::gt:
      r = "gt";
      break;
    case RelOp::lt:
      r = "lt";
      break;
    case RelOp::uge:
      r = "uge";
      break;
    case RelOp::ule:
      r = "ule";
      break;
    case RelOp::ugt:
      r = "ugt";
      break;
    case RelOp::ult:
      r = "ult";
      break;
    default:
      assert(false);
    }
    return r;
  }
};

} // namespace ir