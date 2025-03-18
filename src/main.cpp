#include <iostream>
#include <lexer/lex.hpp>
#include <parser/ast.hpp>
#include <parser/parser.hpp>
#include <parser/pretty_printer.hpp>

#include <seman/analyzer.hpp>
#include <seman/env.hpp>
#include <symbol.hpp>
#include <sysexits.h>

#include <ir/pretty_printer.hpp>
#include <ir/translator.hpp>
#include <seman/escape.hpp>

int main(int argc, char** argv)
{
  std::filesystem::path s = argv[1];

  lexer::Scanner scanner(s);
  lexer::TokenType type;

  symbol::StringTable string_table;
  parser::Parser parser(std::cerr, scanner, string_table);

  std::cerr << "\033[1;31m";
  auto exp = parser.parse();
  if(parser.had_error()) {
    return EX_DATAERR;
  }
  std::cerr << "\033[0m";

  // pretty print the ast
  parser::ast::PrettyPrinter pretty_printer;
  std::cout << exp->accept(pretty_printer) << std::endl << std::endl;

  // dump the string table
  std::cout << "string table:" << std::endl;
  std::cout << string_table.dump() << std::endl;

  // find escape variables
  seman::EscapeFinder esc_finder;
  exp->accept(esc_finder);

  ir::Translator translator;
  seman::Analyzer type_checker(string_table, translator);
  auto ir = type_checker.type_check(*exp);

  ir::PrettyPrinter ir_pretty_printer;
  std::cout << ir->accept(ir_pretty_printer) << std::endl << std::endl;
  return EX_OK;

  /*// test

  std::unique_ptr<ir::Exp> fp = std::make_unique<ir::TempExp>(arch::Frame::FP);
  auto t1 = std::make_unique<ir::BinOpExp>(
    ir::BinaryOp::plus, std::move(fp), std::make_unique<ir::ConstExp>(2));

  auto t2 = std::make_unique<ir::SeqStmt>(
    std::make_unique<ir::ExpStmt>(std::move(t1)),
    std::make_unique<ir::ExpStmt>(std::make_unique<ir::ConstExp>(2)));

  auto t3 = std::make_unique<ir::ESeqExp>(std::move(t2),
                                          std::make_unique<ir::ConstExp>(3));

  auto t4 = std::make_unique<ir::MemExp>(std::move(t3));

  std::vector<std::unique_ptr<ir::Exp>> args;
  args.emplace_back(std::make_unique<ir::ConstExp>(0));

  auto t5 = std::make_unique<ir::CallExp>(
    std::make_unique<ir::NameExp>(ir::Temp::named_label("func")),
    std::move(args));

  auto t6 = std::make_unique<ir::CJumpStmt>(ir::RelOp::lt,
                                            std::move(t4),
                                            std::move(t5),
                                            ir::Temp::named_label("t"),
                                            ir::Temp::named_label("f"));

  ir::PrettyPrinter ir_pretty_printer;
  std::cout << t6->accept(ir_pretty_printer) << std::endl;*/
}