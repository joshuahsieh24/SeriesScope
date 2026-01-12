#include <QApplication>
#include "ui/MainWindow.h"
#include "ui/StyleSheet.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyleSheet(seriesscope::GLOBAL_STYLESHEET);
    seriesscope::MainWindow w;
    w.show();
    return a.exec();
}

