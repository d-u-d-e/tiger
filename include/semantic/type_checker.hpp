#pragma once
#include <parser/ast.hpp>

namespace semantic
{

class TypeChecker {

  TypeChecker() = default;

  void check(const parser::ast::Expression& exp);
};
} // namespace semantic