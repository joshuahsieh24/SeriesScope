#include "ResultsPanel.h"
#include <QVBoxLayout>

namespace seriesscope {
ResultsPanel::ResultsPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Simulation Results"));
    layout->addWidget(new QLabel("Win Prob: --"));
    layout->addWidget(new QLabel("Delta: 0.0%"));
}
}
