#pragma once
#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QProgressBar>
#include <array>
#include "../models/AwardResult.h"
#include "../models/SimulationResult.h"

namespace seriesscope {

class ResultsPanel : public QWidget {
    Q_OBJECT
public:
    explicit ResultsPanel(QWidget* parent = nullptr);

public slots:
    void onResultsReady(const AggregatedResults& results);
    void onAwardsReady(const ss::SeriesAwards& awards);

private:
    QLabel* win_prob_label_   = nullptr;
    QLabel* series_len_label_ = nullptr;
    QGroupBox* awards_box_    = nullptr;

    struct AwardRow {
        QLabel*       badge;
        QLabel*       name;
        QLabel*       rationale;
        QProgressBar* bar;
    };
    std::array<AwardRow, 4> award_rows_;
};

} // namespace seriesscope
