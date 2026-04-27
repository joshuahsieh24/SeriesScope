#include "ResultsPanel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QString>

namespace seriesscope {

static constexpr const char* BADGE_LABELS[4] = { "MVP", "DEF", "CLT", "XFT" };
static constexpr const char* BADGE_STYLE =
    "background-color:#03DAC6; color:#000; font-weight:bold; padding:2px 6px; border-radius:3px;";

ResultsPanel::ResultsPanel(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);

    // --- Simulation results section ---
    auto* sim_box    = new QGroupBox("Simulation Results", this);
    auto* sim_layout = new QVBoxLayout(sim_box);
    win_prob_label_   = new QLabel("Win Probability: --", sim_box);
    series_len_label_ = new QLabel("Most Likely Length: --", sim_box);
    sim_layout->addWidget(win_prob_label_);
    sim_layout->addWidget(series_len_label_);
    root->addWidget(sim_box);

    // --- Awards section ---
    awards_box_       = new QGroupBox("Series Awards", this);
    auto* grid        = new QGridLayout(awards_box_);
    grid->setColumnStretch(2, 1);  // rationale column expands

    for (int i = 0; i < 4; ++i) {
        auto& row      = award_rows_[i];
        row.badge      = new QLabel(BADGE_LABELS[i], awards_box_);
        row.badge->setStyleSheet(BADGE_STYLE);
        row.name       = new QLabel("--", awards_box_);
        row.rationale  = new QLabel("", awards_box_);
        row.bar        = new QProgressBar(awards_box_);
        row.bar->setRange(0, 100);
        row.bar->setValue(0);
        row.bar->setFixedWidth(120);
        row.bar->setStyleSheet(
            "QProgressBar { border:1px solid #444; border-radius:3px; text-align:center; }"
            "QProgressBar::chunk { background:#03DAC6; }");
        grid->addWidget(row.badge,     i, 0);
        grid->addWidget(row.name,      i, 1);
        grid->addWidget(row.rationale, i, 2);
        grid->addWidget(row.bar,       i, 3);
    }
    root->addWidget(awards_box_);
}

void ResultsPanel::onResultsReady(const AggregatedResults& results) {
    double pct = results.team_a_win_pct() * 100.0;
    win_prob_label_->setText(
        QString("Win Probability: %1%").arg(pct, 0, 'f', 1));
    series_len_label_->setText(
        QString("Most Likely Length: %1 games").arg(results.most_likely_length()));
}

void ResultsPanel::onAwardsReady(const ss::SeriesAwards& awards) {
    for (int i = 0; i < 4; ++i) {
        const auto& a  = awards.awards[static_cast<size_t>(i)];
        auto&       row = award_rows_[i];
        row.name->setText(QString::fromStdString(a.player_name));
        row.rationale->setText(QString::fromStdString(a.rationale));
        row.bar->setValue(static_cast<int>(a.score * 100.0));
    }
}

} // namespace seriesscope
