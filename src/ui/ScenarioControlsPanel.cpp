#include "ScenarioControlsPanel.h"
#include <QVBoxLayout>
#include <QLabel>

namespace seriesscope {
ScenarioControlsPanel::ScenarioControlsPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Scenario Modifiers"));
    layout->addWidget(new QSlider(Qt::Horizontal));
    layout->addWidget(new QSlider(Qt::Horizontal));
    layout->addWidget(new QPushButton("Save Baseline"));
}
}
