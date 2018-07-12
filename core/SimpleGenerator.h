#ifndef CHAOSKIT_CORE_SIMPLEGENERATOR_H
#define CHAOSKIT_CORE_SIMPLEGENERATOR_H

#include <stdx/optional.h>
#include <vector>

#include "SimpleInterpreter.h"
#include "System.h"

namespace chaoskit {
namespace core {

class SimpleGenerator {
 public:
  SimpleGenerator(const System &system, uint32_t width, uint32_t height,
                  std::shared_ptr<Rng> rng);
  SimpleGenerator(const System &system, uint32_t width, uint32_t height);

  void setSystem(const System &system);
  void setSize(uint32_t width, uint32_t height);
  void setIterationCount(uint32_t count);
  void setInfiniteIterationCount();

  const float *data() const {
    return buffer_.data();
  }

  void clear();
  void run();

 private:
  uint32_t width_, height_;
  std::vector<float> buffer_;
  stdx::optional<uint32_t> iteration_count_;
  SimpleInterpreter interpreter_;
  std::shared_ptr<Rng> rng_;

  void add(const Point &point);
  void add(uint32_t x, uint32_t y, float factor = 1);
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_SIMPLEGENERATOR_H
