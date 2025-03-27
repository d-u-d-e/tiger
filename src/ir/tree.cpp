#include <cassert>
#include <ir/tree.hpp>
#include <utility>

namespace ir::tree
{
RelOp not_relop(RelOp op)
{
  switch(op) {
  case RelOp::eq:
    return RelOp::ne;
  case RelOp::ne:
    return RelOp::eq;
  case RelOp::lt:
    return RelOp::ge;
  case RelOp::gt:
    return RelOp::le;
  case RelOp::le:
    return RelOp::gt;
  case RelOp::ge:
    return RelOp::lt;
  case RelOp::ult:
    return RelOp::uge;
  case RelOp::ule:
    return RelOp::ugt;
  case RelOp::ugt:
    return RelOp::ule;
  case RelOp::uge:
    return RelOp::ult;
  default:
    assert(false);
  }
  std::unreachable();
}

} // namespace ir::tree