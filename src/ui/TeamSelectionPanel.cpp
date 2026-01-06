#include "TeamSelectionPanel.h"
#include <QVBoxLayout>
#include <QLabel>

namespace seriesscope {
TeamSelectionPanel::TeamSelectionPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Select Teams"));
    layout->addWidget(new QListWidget());
}
}
