#pragma once
#include <QString>

namespace seriesscope {
const QString GLOBAL_STYLESHEET = R"(
    QWidget {
        background-color: #121212;
        color: #E0E0E0;
        font-family: "Segoe UI", sans-serif;
    }
    QPushButton {
        background-color: #1F1F1F;
        border: 1px solid #333;
        padding: 8px;
        border-radius: 4px;
    }
    QPushButton:hover {
        background-color: #333;
    }
    QSlider::handle:horizontal {
        background: #03DAC6;
        width: 18px;
    }
)";
}
