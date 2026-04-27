#pragma once

#include <string>

namespace ss {

struct PlayerProfile {
    std::string player_id;
    std::string player_name;
    std::string team_id;

    // Base stats (per game)
    double minutes = 0.0;
    double pts     = 0.0;
    double stl     = 0.0;
    double blk     = 0.0;
    int    gp      = 0;

    // Advanced stats
    double usg_pct    = 0.0;
    double off_rating = 0.0;
    double def_rating = 0.0;
    double ts_pct     = 0.0;
    double ast_pct    = 0.0;
    double dreb_pct   = 0.0;
    double stl_pct    = 0.0;
    double blk_pct    = 0.0;

    // Clutch stats
    double clutch_min         = 0.0;
    double clutch_pts         = 0.0;
    double clutch_plus_minus  = 0.0;
};

} // namespace ss

namespace seriesscope {
using PlayerProfile = ss::PlayerProfile;
}
