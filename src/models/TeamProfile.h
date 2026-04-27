#pragma once

#include <string>

namespace ss {

struct TeamProfile {
    std::string id;
    std::string name;
    std::string abbreviation;
    double off_rating = 0.0;
    double def_rating = 0.0;
    double net_rating = 0.0;
    double eFG_pct    = 0.0;
    double TOV_pct    = 0.0;
    double ORB_pct    = 0.0;
    double FT_rate    = 0.0;
    std::string primary_color;

    int    wins         = 0;
    int    losses       = 0;
    double recent_form  = 0.0;
    double star_impact  = 1.0;
    double volatility   = 0.20;
    double depth_rating = 0.50;
};

} // namespace ss

namespace seriesscope {
using TeamProfile = ss::TeamProfile;
}
