#include "GameProbabilityModel.h"
#include <cmath>

namespace seriesscope {
double GameProbabilityModel::calculateHomeWinProb(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config) {
    // Basic logit model based on net rating difference
    double home_rating = home.off_rating - home.def_rating;
    double away_rating = away.off_rating - away.def_rating;
    
    // Add home court advantage (approx 3.0 points)
    double diff = (home_rating - away_rating + 3.0) / 10.0;
    
    // Apply modifiers
    diff *= config.offensive_modifier;
    
    return 1.0 / (1.0 + std::exp(-diff));
}
}
