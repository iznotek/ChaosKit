#include "SimpleHistogramGenerator.h"
#include "ThreadLocalRng.h"
#include "toSource.h"

namespace chaoskit::core {

SimpleHistogramGenerator::SimpleHistogramGenerator(const System &system,
                                                   uint32_t width,
                                                   uint32_t height, int ttl,
                                                   std::shared_ptr<Rng> rng)
    : width_(width),
      height_(height),
      buffer_(width * height),
      iteration_count_(stdx::nullopt),
      interpreter_(toSource(system), ttl, Params::fromSystem(system)),
      color_map_(nullptr),
      rng_(std::move(rng)) {}

SimpleHistogramGenerator::SimpleHistogramGenerator(const System &system,
                                                   uint32_t width,
                                                   uint32_t height, int ttl)
    : SimpleHistogramGenerator(system, width, height, ttl,
                               std::make_shared<ThreadLocalRng>()) {}

void SimpleHistogramGenerator::setSystem(const System &system) {
  interpreter_.setSystem(toSource(system));
  interpreter_.setParams(Params::fromSystem(system));
}

void SimpleHistogramGenerator::setTtl(int ttl) { interpreter_.setTtl(ttl); }

void SimpleHistogramGenerator::setSize(uint32_t width, uint32_t height) {
  width_ = width;
  height_ = height;
  buffer_.resize(width * height);
  clear();
}

void SimpleHistogramGenerator::setIterationCount(uint32_t count) {
  iteration_count_ = count;
}

void SimpleHistogramGenerator::setInfiniteIterationCount() {
  iteration_count_ = stdx::nullopt;
}

void SimpleHistogramGenerator::setColorMap(const ColorMap *color_map) {
  color_map_ = color_map;
}

void SimpleHistogramGenerator::clear() {
  std::fill(buffer_.begin(), buffer_.end(), Color::zero());
}

void SimpleHistogramGenerator::run() {
  auto particle = interpreter_.randomizeParticle();

  for (size_t i = 0; !iteration_count_ || i < *iteration_count_; i++) {
    auto [next_state, output] = interpreter_(particle);
    particle = next_state;
    add(output);
  }
}
void SimpleHistogramGenerator::add(const Particle &particle) {
  float x = (particle.x() + 1.f) * (width_ * .5f);
  float y = (particle.y() + 1.f) * (height_ * .5f);

  // Skip the point if out of bounds
  if (x < 0.f || x >= width_ || y < 0.f || y >= height_) {
    return;
  }

  add(static_cast<uint32_t>(x), static_cast<uint32_t>(y), particle.color);
}

void SimpleHistogramGenerator::add(uint32_t x, uint32_t y, float factor) {
  auto &color = buffer_[y * width_ + x];

  if (color_map_) {
    color += color_map_->map(factor);
  } else {
    color += {1, 1, 1, factor};
  }
}

}  // namespace chaoskit::core
