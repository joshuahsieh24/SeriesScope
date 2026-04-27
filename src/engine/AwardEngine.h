#pragma once

#include "models/PlayerProfile.h"
#include "models/AwardResult.h"
#include <vector>
#include <string>

namespace ss {

// Computes 4 deterministic per-series awards from a pool of player stats.
// Qt-free; no RNG dependency — same inputs always produce the same awards.
class AwardEngine {
public:
    // players: top-5 from each team concatenated, identified by team_id.
    // Returns one AwardResult per AwardType, stored at index == AwardType value.
    SeriesAwards computeAwards(
        const std::vector<PlayerProfile>& players,
        const std::string& team_a_id,
        const std::string& team_b_id) const;

private:
    AwardResult computeMVP      (const std::vector<PlayerProfile>& pool) const;
    AwardResult computeDefender (const std::vector<PlayerProfile>& pool) const;
    AwardResult computeClutch   (const std::vector<PlayerProfile>& pool) const;
    AwardResult computeXFactor  (const std::vector<PlayerProfile>& pool) const;

    // Scales raw scores to [0, 1] relative to pool min/max.
    // Returns the score unchanged if all scores are equal (avoids division by zero).
    static double normalize(double val, double minVal, double maxVal);
};

} // namespace ss
