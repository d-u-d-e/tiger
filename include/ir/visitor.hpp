#pragma once
#include <memory>
#include <string>
#include <utility>

namespace ir::tree
{
class Stmt;
class Exp;
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
  std::string virtual visit_const_exp(const ConstExp& exp) = 0;
  std::string virtual visit_name_exp(const NameExp& exp) = 0;
  std::string virtual visit_temp_exp(const TempExp& exp) = 0;
  std::string virtual visit_binop_exp(const BinOpExp& exp) = 0;
  std::string virtual visit_mem_exp(const MemExp& exp) = 0;
  std::string virtual visit_call_exp(const CallExp& exp) = 0;
  std::string virtual visit_eseq_exp(const ESeqExp& exp) = 0;
};

class PrettyPrinterStmtVisitor {
  public:
  std::string virtual visit_move_stmt(const MoveStmt& stmt) = 0;
  std::string virtual visit_exp_stmt(const ExpStmt& stmt) = 0;
  std::string virtual visit_jump_stmt(const JumpStmt& stmt) = 0;
  std::string virtual visit_cjump_stmt(const CJumpStmt& stmt) = 0;
  std::string virtual visit_seq_stmt(const SeqStmt& stmt) = 0;
  std::string virtual visit_label_stmt(const LabelStmt& stmt) = 0;
};

class CanonExprVisitor {
  public:
  std::pair<std::unique_ptr<Stmt>,
            std::unique_ptr<Exp>> virtual visit_const_exp(const ConstExp&
                                                            exp) = 0;

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>> virtual visit_name_exp(
    const NameExp& exp) = 0;

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>> virtual visit_temp_exp(
    const TempExp& exp) = 0;

  std::pair<std::unique_ptr<Stmt>,
            std::unique_ptr<Exp>> virtual visit_binop_exp(const BinOpExp&
                                                            exp) = 0;

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>> virtual visit_mem_exp(
    const MemExp& exp) = 0;

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>> virtual visit_call_exp(
    const CallExp& exp) = 0;

  std::pair<std::unique_ptr<Stmt>, std::unique_ptr<Exp>> virtual visit_eseq_exp(
    const ESeqExp& exp) = 0;
};

class CanonStmtVisitor {
  public:
  std::unique_ptr<Stmt> virtual visit_move_stmt(const MoveStmt& stmt) = 0;
  std::unique_ptr<Stmt> virtual visit_exp_stmt(const ExpStmt& stmt) = 0;
  std::unique_ptr<Stmt> virtual visit_jump_stmt(const JumpStmt& stmt) = 0;
  std::unique_ptr<Stmt> virtual visit_cjump_stmt(const CJumpStmt& stmt) = 0;
  std::unique_ptr<Stmt> virtual visit_seq_stmt(const SeqStmt& stmt) = 0;
  std::unique_ptr<Stmt> virtual visit_label_stmt(const LabelStmt& stmt) = 0;
};

} // namespace ir::tree