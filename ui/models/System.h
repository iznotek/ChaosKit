#ifndef CHAOSKIT_UI_MODELS_SYSTEM_H
#define CHAOSKIT_UI_MODELS_SYSTEM_H

#include <core/Params.h>
#include <QObject>
#include "Blend.h"

namespace chaoskit {
namespace ui {

class System : public QObject {
  Q_OBJECT
 public:
  System(QObject *parent = nullptr);

  const QVector<QPointer<Blend>> &blends() const { return blends_; }
  Blend *addBlend();
  void addBlend(Blend *blend);
  Blend *blendAt(int index) const { return blends_[index]; }
  void removeBlend(Blend *blend);
  void removeBlendAt(int index);

  Blend *finalBlend() const { return final_blend_; }

  core::Params params() const;

 signals:
  void sourceChanged();
  void paramsChanged();
  void finalBlendSourceChanged();
  void finalBlendParamsChanged();

 private:
  QVector<QPointer<Blend>> blends_;
  QPointer<Blend> final_blend_;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_MODELS_SYSTEM_H
