#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace ss {

struct ScenarioConfig {
    std::string home_team_id;
    std::string away_team_id;
    int         series_length        = 7;
    std::string home_schedule_format = "2-2-1-1-1";

    double offensive_modifier  = 1.0;
    double defensive_modifier  = 1.0;
    double volatility_modifier = 1.0;
    std::map<std::string, double> custom_factors;

    double team_a_off_adj       = 0.0;
    double team_b_off_adj       = 0.0;
    double team_a_def_adj       = 0.0;
    double team_b_def_adj       = 0.0;
    double team_a_form_adj      = 0.0;
    double team_b_form_adj      = 0.0;
    double team_a_star_avail    = 1.0;
    double team_b_star_avail    = 1.0;
    double volatility_multiplier = 1.0;
    bool   team_a_has_home_court = true;

    int      num_simulations      = 10000;
    uint64_t rng_seed             = 0xC0FFEE1234ULL;
    int      thread_count_override = 0;
};

} // namespace ss

namespace seriesscope {
using ScenarioConfig = ss::ScenarioConfig;
}
