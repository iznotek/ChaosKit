#include "FormulaPreviewProvider.h"

#include <QImage>
#include <QPainter>
#include <QQuickImageResponse>
#include <QRunnable>
#include <QSize>
#include <memory>
#include <vector>
#include "core/Params.h"
#include "core/Point.h"
#include "core/SimpleInterpreter.h"
#include "core/structures/Blend.h"
#include "core/structures/Formula.h"
#include "core/structures/System.h"
#include "core/toSource.h"
#include "library/FormulaType.h"

namespace chaoskit::ui {

namespace {

constexpr int GRID_WIDTH = 16;
constexpr int GRID_HEIGHT = 16;
constexpr double GRID_EXPANSION_FACTOR = .2;
constexpr int ITERATIONS = 3;

/** Returns a vector of good-looking params for a formula. */
std::vector<float> getParamsForFormula(library::FormulaType type) {
  switch (type) {
    case library::FormulaType::DeJong:
      return {-1.9292301883127383f, -1.7559409159631594f, -1.8413772506711874f,
              -1.972643807513176f};
    case library::FormulaType::Drain:
      return {0.7f, -0.5f, 0.3f, 0.f};
    default:
      return {};
  }
}

core::SimpleInterpreter createInterpreter(library::FormulaType type) {
  auto formula = std::make_unique<core::Formula>();
  formula->setType(type);
  formula->params = getParamsForFormula(type);

  auto blend = std::make_unique<core::Blend>();
  blend->formulas.push_back(formula.get());

  core::System system{};
  system.blends.push_back(blend.get());

  return core::SimpleInterpreter(core::toSource(system),
                                 core::Particle::IMMORTAL,
                                 core::Params::fromSystem(system));
}

QVector<QPointF> generateGrid(library::FormulaType type, int width,
                              int height) {
  core::SimpleInterpreter interpreter = createInterpreter(type);
  QVector<QPointF> points(width * height);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // Convert the point to be in [-1; 1]
      core::Particle particle{{(float)(x * 2) / (float)(width - 1) - 1.f,
                               (float)(y * 2) / (float)(height - 1) - 1.f},
                              0.f,
                              core::Particle::IMMORTAL};

      int index = y * width + x;
      for (int i = 0; i < ITERATIONS; i++) {
        particle = interpreter(particle).next_state;
      }
      points[index].setX(particle.x());
      points[index].setY(particle.y());
    }
  }

  return points;
}

QRectF getBoundsForGrid(const QVector<QPointF>& grid) {
  auto top = std::numeric_limits<qreal>::max();
  auto right = std::numeric_limits<qreal>::min();
  auto bottom = std::numeric_limits<qreal>::min();
  auto left = std::numeric_limits<qreal>::max();

  for (const auto& point : grid) {
    if (point.x() < left) left = point.x();
    if (point.x() > right) right = point.x();
    if (point.y() < top) top = point.y();
    if (point.y() > bottom) bottom = point.y();
  }

  return QRectF(left, top, right - left, bottom - top);
}

class FormulaPreviewResponse : public QQuickImageResponse, public QRunnable {
 public:
  FormulaPreviewResponse(QString type, const QSize& requestedSize)
      : type_(std::move(type)), requestedSize_(requestedSize) {
    setAutoDelete(false);
  }

  void run() override {
    QSize imageSize(200, 200);

    image_ = QImage(imageSize, QImage::Format_RGBA64);
    image_.fill(QColor(0, 0, 0, 0));

    QPainter painter(&image_);
    QColor primary("#fde38a");
    painter.setPen(primary);

    auto formulaType =
        library::FormulaType::_from_string_nothrow(type_.toUtf8());
    if (formulaType) {
      QVector<QPointF> grid =
          generateGrid(*formulaType, GRID_WIDTH, GRID_HEIGHT);

      QRectF bounds = getBoundsForGrid(grid);
      double dx = bounds.width() * GRID_EXPANSION_FACTOR * .5;
      double dy = bounds.height() * GRID_EXPANSION_FACTOR * .5;
      bounds.adjust(-dx, -dy, dx, dy);

      QTransform transform =
          QTransform::fromScale(imageSize.width() / bounds.width(),
                                imageSize.height() / bounds.height())
              .translate(-bounds.left(), -bounds.top());

      for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
          int index = y * GRID_WIDTH + x;

          if (x < GRID_WIDTH - 1) {
            int right = index + 1;
            painter.drawLine(transform.map(grid[index]),
                             transform.map(grid[right]));
          }
          if (y < GRID_HEIGHT - 1) {
            int bottom = index + GRID_WIDTH;
            painter.drawLine(transform.map(grid[index]),
                             transform.map(grid[bottom]));
          }
        }
      }
    }

    if (requestedSize_.isValid()) {
      image_ = image_.scaled(requestedSize_);
    }

    emit finished();
  }

  [[nodiscard]] QQuickTextureFactory* textureFactory() const override {
    return QQuickTextureFactory::textureFactoryForImage(image_);
  }

 private:
  QString type_;
  QImage image_;
  QSize requestedSize_;
};

}  // namespace

QQuickImageResponse* FormulaPreviewProvider::requestImageResponse(
    const QString& id, const QSize& requestedSize) {
  auto* response = new FormulaPreviewResponse(id, requestedSize);
  pool_.start(response);
  return response;
}

}  // namespace chaoskit::ui
