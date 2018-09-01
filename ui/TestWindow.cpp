#include "TestWindow.h"
#include <library/DeJong.h>
#include <QTimer>

using chaoskit::core::Blend;
using chaoskit::core::Formula;
using chaoskit::core::HistogramBuffer;
using chaoskit::core::System;
using chaoskit::core::Transform;
using chaoskit::library::DeJong;

namespace chaoskit {
namespace ui {

TestWindow::TestWindow() {
  setTitle(QStringLiteral("ChaosKit (Test Window)"));
  setBaseSize(QSize(512, 512));

  Formula formula = Formula::build<DeJong>(
      {9.379666578024626e-01f, 1.938709271140397e+00f, -1.580897020176053e-01f,
       -1.430070123635232e+00f});
  Blend final{Transform::identity(),
              Transform::fromScale(.5f, 1.f).translate(0.3f, 0.5f)};
  Blend blend;
  blend.formulas.push_back(formula);
  System system{{blend}, final};

  histogramGenerator_ = new HistogramGenerator(this);
  histogramGenerator_->setSize(512, 512);
  histogramGenerator_->setSystem(system);
  histogramGenerator_->setTtl(20);
  toneMapper_ = new GLToneMapper(this);

  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(format);

  connect(this, &QOpenGLWindow::frameSwapped, this, &TestWindow::syncHistogram);
}

void TestWindow::initializeGL() {
  toneMapper_->initializeGL();
  histogramGenerator_->start();
}

void TestWindow::resizeGL(int w, int h) {
  const qreal pixelRatio = devicePixelRatio();
  auto *fns = context()->versionFunctions<QOpenGLFunctions_3_2_Core>();

  fns->glViewport(0, 0, static_cast<GLsizei>(w * pixelRatio),
                  static_cast<GLsizei>(h * pixelRatio));
}

void TestWindow::paintGL() { toneMapper_->map(); }

void TestWindow::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
    case Qt::Key_R:
      histogramGenerator_->clear();
      break;
    case Qt::Key_Escape:
      close();
      break;
    default:
      return;
  }
}

void TestWindow::syncHistogram() {
  histogramGenerator_->withHistogram([this](const HistogramBuffer &histogram) {
    toneMapper_->syncBuffer(histogram);
  });
  update();
}

}  // namespace ui
}  // namespace chaoskit