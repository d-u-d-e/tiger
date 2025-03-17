#pragma once
#include <string>

namespace ir
{
struct ConstExp;
struct NameExp;
struct TempExp;
struct BinOpExp;
struct MemExp;
struct CallExp;
struct ESeqExp;
struct MoveStmt;
struct ExpStmt;
struct JumpStmt;
struct CJumpStmt;
struct SeqStmt;
struct LabelStmt;

class PrettyPrinterExprVisitor {
  public:
  // clang-format off
  std::string virtual visit_const_exp(const ConstExp& exp) = 0;
  std::string virtual visit_name_exp(const NameExp& exp) = 0;
  std::string virtual visit_temp_exp(const TempExp& exp) = 0;
  std::string virtual visit_binop_exp(const BinOpExp& exp) = 0;
  std::string virtual visit_mem_exp(const MemExp& exp) = 0;
  std::string virtual visit_call_exp(const CallExp& exp) = 0;
  std::string virtual visit_eseq_exp(const ESeqExp& exp) = 0;
  // clang-format on
};

class PrettyPrinterStmtVisitor {
  public:
  // clang-format off
  std::string virtual visit_move_stmt(const MoveStmt& stmt) = 0;
  std::string virtual visit_exp_stmt(const ExpStmt& stmt) = 0;
  std::string virtual visit_jump_stmt(const JumpStmt& stmt) = 0;
  std::string virtual visit_cjump_stmt(const CJumpStmt& stmt) = 0;
  std::string virtual visit_seq_stmt(const SeqStmt& stmt) = 0;
  std::string virtual visit_label_stmt(const LabelStmt& stmt) = 0;
  // clang-format on
};

} // namespace ir