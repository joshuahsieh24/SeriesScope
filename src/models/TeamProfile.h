#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace seriesscope {
struct TeamProfile {
    std::string id;
    std::string name;
    std::string abbreviation;
    double off_rating;
    double def_rating;
    double net_rating;
    double eFG_pct;
    double TOV_pct;
    double ORB_pct;
    double FT_rate;
    std::string primary_color;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TeamProfile, id, name, abbreviation, off_rating, def_rating, net_rating, eFG_pct, TOV_pct, ORB_pct, FT_rate, primary_color)
};
}
