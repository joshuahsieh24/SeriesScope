#pragma once
#include "../models/TeamProfile.h"
#include "../models/ScenarioConfig.h"
#include "../models/SimulationResult.h"
#include <random>

namespace seriesscope {
class SeriesSimulator {
public:
    SeriesSimulator(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config);
    SimulationResult simulateOneSeries(std::mt19937& gen);
private:
    TeamProfile home, away;
    ScenarioConfig config;
};
}
