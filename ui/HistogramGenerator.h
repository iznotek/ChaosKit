#ifndef CHAOSKIT_UI_HISTOGRAMGENERATOR_H
#define CHAOSKIT_UI_HISTOGRAMGENERATOR_H

#include <QObject>
#include <QThread>
#include "BlenderTask.h"
#include "GathererTask.h"
#include "HistogramBuffer.h"

namespace chaoskit::ui {

class HistogramGenerator : public QObject {
  Q_OBJECT
 public:
  explicit HistogramGenerator(QObject *parent = nullptr);
  ~HistogramGenerator() override;

  void withHistogram(
      const std::function<void(const core::HistogramBuffer &)> &action);

  [[nodiscard]] bool running() const { return running_; }

 public slots:
  void setSystem(const chaoskit::core::System *system);
  void setColorMap(const chaoskit::core::ColorMap *colorMap);
  void setSize(quint32 width, quint32 height);
  void setTtl(int32_t ttl);
  void start();
  void stop();
  void clear();

 signals:
  void started();
  void stopped();

 private:
  QThread *thread_;
  BlenderTask *blenderTask_;
  GathererTask *gathererTask_;
  bool running_ = false;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_HISTOGRAMGENERATOR_H
