#pragma once
#include "../models/TeamProfile.h"
#include "../models/ScenarioConfig.h"

namespace seriesscope {
class GameProbabilityModel {
public:
    static double calculateHomeWinProb(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config);
};
}
