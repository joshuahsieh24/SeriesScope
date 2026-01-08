#pragma once
#include <QWidget>
#include <QSlider>
#include <QPushButton>

namespace seriesscope {
class ScenarioControlsPanel : public QWidget {
    Q_OBJECT
public:
    ScenarioControlsPanel(QWidget* parent = nullptr);
};
}
