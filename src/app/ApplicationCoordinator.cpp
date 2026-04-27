#include "ApplicationCoordinator.h"
#include "../engine/MonteCarloRunner.h"

namespace seriesscope {

void ApplicationCoordinator::runSimulation(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config) {
    QtConcurrent::run([=]() {
        auto results = MonteCarloRunner::run(home, away, config, 10000);
        emit resultsReady(results);
    });
}

void ApplicationCoordinator::computeAwards(const std::string& team_a_id, const std::string& team_b_id) {
    auto players = player_repo_.getPlayersForMatchup(team_a_id, team_b_id);
    auto awards  = award_engine_.computeAwards(players, team_a_id, team_b_id);
    emit awardsReady(awards);
}

}
