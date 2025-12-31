#include "ApplicationCoordinator.h"
#include "../engine/MonteCarloRunner.h"

namespace seriesscope {
void ApplicationCoordinator::runSimulation(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config) {
    QtConcurrent::run([=]() {
        auto results = MonteCarloRunner::run(home, away, config, 10000);
        emit resultsReady(results);
    });
}
}
