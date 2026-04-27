#pragma once

#include <string>
#include <array>

namespace ss {

enum class AwardType {
    SeriesMVP    = 0,
    BestDefender = 1,
    ClutchPlayer = 2,
    XFactor      = 3
};

struct AwardResult {
    AwardType   award;
    std::string player_name;
    std::string team_id;
    double      score     = 0.0;  // normalized [0, 1] for progress bar display
    std::string rationale;        // one-line explanation shown in UI
};

struct SeriesAwards {
    std::array<AwardResult, 4> awards;

    const AwardResult& mvp()      const { return awards[0]; }
    const AwardResult& defender() const { return awards[1]; }
    const AwardResult& clutch()   const { return awards[2]; }
    const AwardResult& xfactor()  const { return awards[3]; }
};

} // namespace ss

namespace seriesscope {
using AwardResult  = ss::AwardResult;
using SeriesAwards = ss::SeriesAwards;
using AwardType    = ss::AwardType;
}
