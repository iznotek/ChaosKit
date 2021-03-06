#include "Input.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(Input)

std::ostream &operator<<(std::ostream &stream, const Input &input) {
  return stream << node_type(input) << "(" << input.type() << ")";
}

}  // namespace chaoskit::ast
