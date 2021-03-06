#ifndef CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H
#define CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H

#include <stdx/optional.h>
#include <vector>

#include "Color.h"
#include "ColorMap.h"
#include "SimpleInterpreter.h"
#include "structures/System.h"

namespace chaoskit::core {

class SimpleHistogramGenerator {
 public:
  SimpleHistogramGenerator(const System &system, uint32_t width,
                           uint32_t height, int ttl, std::shared_ptr<Rng> rng);
  SimpleHistogramGenerator(const System &system, uint32_t width,
                           uint32_t height, int ttl = Particle::IMMORTAL);

  void setSystem(const System &system);
  void setSize(uint32_t width, uint32_t height);
  void setTtl(int ttl);
  void setColorMap(const ColorMap *color_map);
  void setIterationCount(uint32_t count);
  void setInfiniteIterationCount();

  [[nodiscard]] const Color *data() const { return buffer_.data(); }

  void clear();
  void run();

 private:
  uint32_t width_, height_;
  std::vector<Color> buffer_;
  stdx::optional<uint32_t> iteration_count_;
  SimpleInterpreter interpreter_;
  const ColorMap *color_map_;
  std::shared_ptr<Rng> rng_;

  void add(const Particle &particle);
  void add(uint32_t x, uint32_t y, float factor = 1);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H
