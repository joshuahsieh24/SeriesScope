#pragma once
#include <QObject>
#include <QtConcurrent>
#include "../models/TeamProfile.h"
#include "../models/ScenarioConfig.h"
#include "../models/SimulationResult.h"
#include "../models/AwardResult.h"
#include "../engine/AwardEngine.h"
#include "../data/DatabaseManager.h"
#include "../data/PlayerRepository.h"
#include <string>

namespace seriesscope {
class ApplicationCoordinator : public QObject {
    Q_OBJECT
public:
    explicit ApplicationCoordinator(DatabaseManager& db, QObject* parent = nullptr)
        : QObject(parent), player_repo_(db) {}

    void runSimulation(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config);

    // Synchronous — O(10) arithmetic, runs on the calling thread.
    // Emit awardsReady immediately; call this right after runSimulation.
    void computeAwards(const std::string& team_a_id, const std::string& team_b_id);

signals:
    void resultsReady(AggregatedResults results);
    void awardsReady(ss::SeriesAwards awards);

private:
    ss::AwardEngine      award_engine_;
    PlayerRepository     player_repo_;
};
}
