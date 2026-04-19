#pragma once
#include "models/TeamProfile.h"
#include "models/ScenarioConfig.h"

namespace ss {

// Computes P(Team A wins a single game) using an explicit, calibrated formula.
//
// Model design:
//
//   total_pts = net_margin   × PACE_SCALE      [efficiency edge, per-game units]
//             + wpc_margin   × WIN_PCT_SCALE    [execution/clutch signal]
//             + form_margin  × FORM_SCALE       [momentum, last 10 games]
//             ± HOME_COURT_PTS                  [±3.2 pt swing, symmetric]
//             + star_penalty                    [availability modeled as a penalty]
//
//   P(A wins) = logistic(total_pts / LOGISTIC_SCALE)
//
// Calibration anchors:
//   HOME_COURT_PTS = 3.2  → logistic(3.2/7.0) ≈ 0.618, matching historical NBA
//                           home-court win rate (~60-63%).
//   LOGISTIC_SCALE = 7.0  → a 7-pt edge maps to ~73% win prob; consistent with
//                           NBA point-spread research (Hollinger, 538).
//   PACE_SCALE     = 0.50 → net ratings are per-100 possessions; ~94 poss/game.
//                           0.50 dampens noise and keeps the logit well-conditioned.
//   FORM_SCALE     = 1.5  → max form swing ≈ half an HCA (±1.5 pts).
//   WIN_PCT_SCALE  = 1.0  → supplemental clutch/execution signal; subordinate to
//                           efficiency ratings.
//   STAR_VALUE_PTS = 4.0  → full star unavailability penalty; calibrated from
//                           Win Share production of franchise-level stars.
//
// The model is stateless and free-threaded.
class GameProbabilityModel {
public:
    static constexpr double HOME_COURT_PTS  = 3.2;
    static constexpr double LOGISTIC_SCALE  = 7.0;
    static constexpr double PACE_SCALE      = 0.50;
    static constexpr double FORM_SCALE      = 1.5;
    static constexpr double WIN_PCT_SCALE   = 1.0;
    static constexpr double STAR_VALUE_PTS  = 4.0;

    // Compute P(Team A wins this game).
    double computeWinProbability(
        const TeamProfile&    a,
        const TeamProfile&    b,
        const ScenarioConfig& cfg,
        bool                  a_is_home) const;

private:
    static double logistic(double x);
};

} // namespace ss
