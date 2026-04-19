#include "engine/GameProbabilityModel.h"
#include <cmath>
#include <algorithm>

namespace ss {

// static
double GameProbabilityModel::logistic(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

double GameProbabilityModel::computeWinProbability(
    const TeamProfile&    a,
    const TeamProfile&    b,
    const ScenarioConfig& cfg,
    bool                  a_is_home) const
{
    // ── Component 1: Efficiency margin ────────────────────────────────────────
    // Net rating is an opponent-adjusted per-100-possession efficiency metric.
    // The difference is the best single predictor of head-to-head game outcomes.
    // Scenario sliders let the user override the offensive/defensive contribution.
    double net_a = a.net_rating + cfg.team_a_off_adj - cfg.team_a_def_adj;
    double net_b = b.net_rating + cfg.team_b_off_adj - cfg.team_b_def_adj;
    double eff_pts = (net_a - net_b) * PACE_SCALE;

    // ── Component 2: Win percentage (execution signal) ─────────────────────
    // Win% captures clutch execution, depth, and regular-season consistency
    // that net rating does not fully account for.  Laplace smoothing avoids
    // extreme values for teams with few games.
    double wpc_a = (a.wins + 0.5) / (a.wins + a.losses + 1.0);
    double wpc_b = (b.wins + 0.5) / (b.wins + b.losses + 1.0);
    double wpc_pts = (wpc_a - wpc_b) * WIN_PCT_SCALE;

    // ── Component 3: Recent form (momentum) ───────────────────────────────
    // recent_form ∈ [−1, 1]: (wins_last10 − 5) / 5. Scaled by FORM_SCALE so
    // the maximum form swing is ≈ half a home-court advantage.
    double form_a = a.recent_form + cfg.team_a_form_adj;
    double form_b = b.recent_form + cfg.team_b_form_adj;
    double form_pts = (form_a - form_b) * FORM_SCALE;

    // ── Component 4: Home court (symmetric) ──────────────────────────────
    // HOME_COURT_PTS applied symmetrically — home team gains exactly as much
    // as the away team loses.  Calibrated to match the NBA's ~61% historical
    // home win rate.
    double home_pts = a_is_home ? HOME_COURT_PTS : -HOME_COURT_PTS;

    // ── Component 5: Star availability (penalty model) ───────────────────
    // Modeled as a penalty for unavailability rather than a bonus for presence.
    // A fully healthy star contributes 0 to this term; unavailability subtracts
    // STAR_VALUE_PTS × star_impact × (1 − availability).
    double star_pts =
        -(1.0 - cfg.team_a_star_avail) * a.star_impact * STAR_VALUE_PTS
        +(1.0 - cfg.team_b_star_avail) * b.star_impact * STAR_VALUE_PTS;

    double total = eff_pts + wpc_pts + form_pts + home_pts + star_pts;
    double p     = logistic(total / LOGISTIC_SCALE);

    // Clamp to avoid degenerate probabilities from extreme inputs.
    return std::clamp(p, 0.02, 0.98);
}

} // namespace ss
