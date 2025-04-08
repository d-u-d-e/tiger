#pragma once
#include <cassert>
#include <cstddef>
#include <format>
#include <ir/tree.hpp>
#include <memory>
#include <string>
#include <variant>

namespace ir::tree
{

class PrettyPrinter {

  public:
  std::string operator()(const std::unique_ptr<ConstExp>& exp)
  {
    return std::format("{}ConstExp({})", indent(), exp->v);
  }

  std::string operator()(const std::unique_ptr<NameExp>& exp)
  {
    return std::format("{}NameExp({})", indent(), exp->label.str());
  }

  std::string operator()(const std::unique_ptr<TempExp>& exp)
  {
    return std::format("{}TempExp(t{})", indent(), exp->temp);
  }

  std::string operator()(const std::unique_ptr<BinOpExp>& exp)
  {
    std::string r = indent() + "BinOpExp(\n";
    depth++;
    r += indent() + name(exp->op) + ",\n";
    r += std::visit(*this, exp->left) + ",\n";
    r += std::visit(*this, exp->right) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string operator()(const std::unique_ptr<MemExp>& exp)
  {
    std::string r = indent() + "MemExp(\n";
    depth++;
    r += std::visit(*this, exp->a) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string operator()(const std::unique_ptr<CallExp>& exp)
  {
    std::string r = indent() + "CallExpr(\n";
    depth++;
    r += std::visit(*this, exp->fun) + ",\n";
    r += indent() + "[\n";
    auto size = exp->args.size();
    depth++;
    for(size_t i = 0; i < size; i++)
    {
      auto& arg = exp->args[i];
      r += std::visit(*this, arg) + ((i == size - 1) ? "\n" : ",\n");
    }
    depth--;
    r += indent() + "]\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string operator()(const std::unique_ptr<ESeqExp>& exp)
  {
    std::string r = indent() + "ESeq(\n";
    depth++;
    r += std::visit(*this, exp->stmt) + ",\n";
    r += std::visit(*this, exp->exp) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string operator()(const std::unique_ptr<MoveStmt>& stmt)
  {
    std::string r = indent() + "MoveStmt(\n";
    depth++;
    r += std::visit(*this, stmt->left) + ",\n";
    r += std::visit(*this, stmt->right) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string operator()(const std::unique_ptr<ExpStmt>& stmt)
  {
    std::string r = indent() + "ExpStmt(\n";
    depth++;
    r += std::visit(*this, stmt->exp) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string operator()(const std::unique_ptr<JumpStmt>& stmt)
  {
    std::string r = indent() + "JumpStmt(\n";
    depth++;
    r += std::visit(*this, stmt->a) + ",\n";
    r += indent() + "[";
    auto size = stmt->labels.size();
    for(size_t i = 0; i < size; i++)
    {
      auto& l = stmt->labels[i];
      r += l.str() + ((i == size - 1) ? "]\n" : ", ");
    }
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string operator()(const std::unique_ptr<CJumpStmt>& stmt)
  {
    std::string r = indent() + "CondJumpStmt(\n";
    depth++;
    r += indent() + name(stmt->op) + ",\n";
    r += indent() + stmt->tlabel.str() + ",\n";
    r += indent() + stmt->flabel.str() + ",\n";
    r += std::visit(*this, stmt->lexp) + ",\n";
    r += std::visit(*this, stmt->rexp) + "\n";
    depth--;
    r += indent() + ")";
    return r;
  }

  std::string operator()(const std::unique_ptr<SeqStmt>& stmt)
  {
    std::string r = indent() + "SeqStmt[\n";
    depth++;
    r += std::visit(*this, stmt->stm1) + ",\n";
    r += std::visit(*this, stmt->stm2) + "\n";
    depth--;
    r += indent() + "]";
    return r;
  }

  std::string operator()(const std::unique_ptr<LabelStmt>& stmt)
  {
    return std::format("{}LabelStmt({})", indent(), stmt->label.str());
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
    switch(op)
    {
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
    switch(op)
    {
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

} // namespace ir::tree