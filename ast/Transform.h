#ifndef CHAOSKIT_AST_TRANSFORM_H
#define CHAOSKIT_AST_TRANSFORM_H

#include <array>

namespace chaoskit {
namespace ast {

class Transform {
 public:
  Transform() : params_{1.f, 0.f, 0.f, 0.f, 1.f, 0.f} {}
  Transform(const float(params)[6])
      : params_{params[0], params[1], params[2],
                params[3], params[4], params[5]} {}
  Transform(float a, float b, float c, float d, float e, float f)
      : params_{a, b, c, d, e, f} {}

  const std::array<float, 6> &params() const { return params_; }

  bool operator==(const Transform &other) const {
    return params_ == other.params_;
  }

 private:
  std::array<float, 6> params_;
};

std::ostream &operator<<(std::ostream &stream, const Transform &transform);

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_AST_TRANSFORM_H
