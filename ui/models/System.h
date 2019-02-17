#ifndef CHAOSKIT_UI_MODELS_SYSTEM_H
#define CHAOSKIT_UI_MODELS_SYSTEM_H

#include <core/Params.h>
#include <QObject>
#include "Blend.h"

namespace chaoskit {
namespace ui {
namespace models {

class System : public QObject {
  Q_OBJECT
 public:
  System(QObject *parent = nullptr);

  const QVector<QPointer<Blend>> &blends() const { return blends_; }
  Blend *addBlend();
  void addBlend(Blend *blend);

  const Blend *finalBlend() const { return final_blend_; }
  void setFinalBlend(Blend *blend);

  core::Params params() const;

 signals:
  void blendsChanged();
  void finalBlendChanged();

 private:
  QVector<QPointer<Blend>> blends_;
  QPointer<Blend> final_blend_;
};

}  // namespace models
}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_MODELS_SYSTEM_H