#pragma once
#include <QWidget>
#include <QLabel>

namespace seriesscope {
class ResultsPanel : public QWidget {
    Q_OBJECT
public:
    ResultsPanel(QWidget* parent = nullptr);
};
}
