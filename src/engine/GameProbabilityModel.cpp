#include "GameProbabilityModel.h"
#include <cmath>

namespace seriesscope {
double GameProbabilityModel::calculateHomeWinProb(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config) {
    // Calibrated formula
    double elo_diff = (home.off_rating - home.def_rating) - (away.off_rating - away.def_rating);
    double hca = 3.5;
    return 1.0 / (1.0 + std::pow(10.0, -(elo_diff + hca) / 10.0));
}
}
