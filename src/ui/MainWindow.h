#pragma once
#include <QMainWindow>
#include <QSplitter>

namespace seriesscope {
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
};
}
