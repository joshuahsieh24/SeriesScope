#pragma once
#include <string>
#include <map>

namespace seriesscope {
struct ScenarioConfig {
    std::string home_team_id;
    std::string away_team_id;
    int series_length = 7;
    std::string home_schedule_format = "2-2-1-1-1";
    
    // Sliders/Modifiers
    double offensive_modifier = 1.0;
    double defensive_modifier = 1.0;
    double volatility_modifier = 1.0;
    
    std::map<std::string, double> custom_factors;
};
}
