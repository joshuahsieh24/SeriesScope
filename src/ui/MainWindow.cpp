#include "MainWindow.h"
#include <QStatusBar>
#include <QLabel>

namespace seriesscope {
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("SeriesScope Pro");
    resize(1200, 800);
    
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);
    
    mainSplitter->addWidget(new QWidget()); // Team Selection
    mainSplitter->addWidget(new QWidget()); // Controls
    mainSplitter->addWidget(new QWidget()); // Results
    
    statusBar()->addPermanentWidget(new QLabel("Data as of: 2024-04-19"));
}
}
