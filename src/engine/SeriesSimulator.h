#pragma once

#include "engine/GameProbabilityModel.h"
#include "models/ScenarioConfig.h"
#include "models/SimulationResult.h"
#include "models/TeamProfile.h"

#include <random>

namespace ss {

class SeriesSimulator {
public:
    explicit SeriesSimulator(const GameProbabilityModel& model);

    SimulationResult simulateOnce(
        const TeamProfile&    a,
        const TeamProfile&    b,
        const ScenarioConfig& cfg,
        std::mt19937_64&      rng) const;

private:
    const GameProbabilityModel& model_;
};

} // namespace ss
