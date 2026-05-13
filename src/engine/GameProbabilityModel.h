#pragma once
#include "models/TeamProfile.h"
#include "models/ScenarioConfig.h"

namespace ss {

// Computes P(Team A wins a single game) using an explicit, calibrated formula.
//
// Model design:
//
//   total_pts = net_margin        × PACE_SCALE           [efficiency edge]
//             + wpc_margin        × WIN_PCT_SCALE         [execution/clutch signal]
//             + form_margin       × FORM_SCALE            [momentum, last 10 games]
//             ± HOME_COURT_PTS                            [±3.2 pt swing, symmetric]
//             + star_penalty                              [availability penalty]
//             + four_factors_delta × FOUR_FACTORS_SCALE   [Dean Oliver Four Factors]
//
//   four_factors_delta = (eFG_a−eFG_b)×0.40 + (TOV_b−TOV_a)×0.25
//                      + (ORB_a−ORB_b)×0.20 + (FT_a−FT_b)×0.15
//
//   P(A wins) = logistic(total_pts / LOGISTIC_SCALE)
//
// Calibration anchors:
//   HOME_COURT_PTS    = 3.2  → logistic(3.2/7.0) ≈ 0.618, matching historical NBA HCA.
//   LOGISTIC_SCALE    = 7.0  → a 7-pt edge maps to ~73% win prob.
//   PACE_SCALE        = 0.50 → dampens per-100 net ratings to per-game units.
//   FORM_SCALE        = 1.5  → max form swing ≈ half an HCA.
//   WIN_PCT_SCALE     = 1.0  → supplemental clutch/execution signal.
//   STAR_VALUE_PTS    = 4.0  → full star unavailability penalty.
//   FOUR_FACTORS_SCALE = 10.0 → a 0.02 gap in each factor contributes ~0.2 pts,
//                               subordinate to net rating but not negligible.
//
// The model is stateless and free-threaded.
class GameProbabilityModel {
public:
    static constexpr double HOME_COURT_PTS     = 3.2;
    static constexpr double LOGISTIC_SCALE     = 7.0;
    static constexpr double PACE_SCALE         = 0.50;
    static constexpr double FORM_SCALE         = 1.5;
    static constexpr double WIN_PCT_SCALE      = 1.0;
    static constexpr double STAR_VALUE_PTS     = 4.0;
    static constexpr double FOUR_FACTORS_SCALE = 10.0;

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
