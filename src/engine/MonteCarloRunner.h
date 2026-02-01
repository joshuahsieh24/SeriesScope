#pragma once
#include "../models/TeamProfile.h"
#include "../models/ScenarioConfig.h"
#include "../models/SimulationResult.h"

namespace seriesscope {
class MonteCarloRunner {
public:
    static AggregatedResults run(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config, int iterations);
};
}
