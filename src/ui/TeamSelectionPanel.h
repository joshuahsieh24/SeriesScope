#pragma once
#include <QWidget>
#include <QListWidget>

namespace seriesscope {
class TeamSelectionPanel : public QWidget {
    Q_OBJECT
public:
    TeamSelectionPanel(QWidget* parent = nullptr);
};
}
