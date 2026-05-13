#include "AwardEngine.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace ss {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string fmt1(double v) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(1) << v;
    return s.str();
}

static std::string fmt0(double v) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(0) << v;
    return s.str();
}

double AwardEngine::normalize(double val, double minVal, double maxVal) {
    if (maxVal <= minVal) return 1.0;
    return (val - minVal) / (maxVal - minVal);
}

template <typename Scorer>
std::pair<std::size_t, double> AwardEngine::selectWinner(
    const std::vector<PlayerProfile>& pool,
    Scorer scorer) const
{
    std::vector<double> scores;
    scores.reserve(pool.size());
    for (const auto& p : pool) scores.push_back(scorer(p));

    auto it  = std::max_element(scores.begin(), scores.end());
    auto idx = static_cast<std::size_t>(it - scores.begin());
    double mn = *std::min_element(scores.begin(), scores.end());
    return { idx, normalize(scores[idx], mn, *it) };
}

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------

SeriesAwards AwardEngine::computeAwards(
    const std::vector<PlayerProfile>& players,
    const std::string& /*team_a_id*/,
    const std::string& /*team_b_id*/) const
{
    SeriesAwards out;
    out.awards[0] = computeMVP(players);
    out.awards[1] = computeDefender(players);
    out.awards[2] = computeClutch(players);
    out.awards[3] = computeXFactor(players);
    return out;
}

// ---------------------------------------------------------------------------
// MVP: usage × efficiency × offensive impact
// Formula: usg_pct×0.35 + ts_pct×0.35 + (off_rating/130.0)×0.30
// ---------------------------------------------------------------------------

AwardResult AwardEngine::computeMVP(const std::vector<PlayerProfile>& pool) const {
    auto [idx, score] = selectWinner(pool, [](const PlayerProfile& p) {
        return p.usg_pct * 0.35
             + p.ts_pct  * 0.35
             + (p.off_rating / 130.0) * 0.30;
    });
    const auto& w = pool[idx];
    return { AwardType::SeriesMVP, w.player_name, w.team_id, score,
             w.player_name + " — "
             + fmt0(w.usg_pct * 100) + "% USG, "
             + fmt0(w.ts_pct  * 100) + "% TS, "
             + fmt0(w.off_rating)    + " ORtg" };
}

// ---------------------------------------------------------------------------
// Best Defender: defensive rating + steal/block/reb rates
// Formula: stl_pct×0.25 + blk_pct×0.25 + dreb_pct×0.20 + ((130-def_rating)/40)×0.30
// Lower def_rating is better; (130-x)/40 converts to a positive signal.
// ---------------------------------------------------------------------------

AwardResult AwardEngine::computeDefender(const std::vector<PlayerProfile>& pool) const {
    auto [idx, score] = selectWinner(pool, [](const PlayerProfile& p) {
        return p.stl_pct  * 0.25
             + p.blk_pct  * 0.25
             + p.dreb_pct * 0.20
             + ((130.0 - p.def_rating) / 40.0) * 0.30;
    });
    const auto& w = pool[idx];
    return { AwardType::BestDefender, w.player_name, w.team_id, score,
             w.player_name + " — "
             + fmt0(w.def_rating)           + " DRtg, "
             + fmt1(w.stl_pct * 100)        + "% STL, "
             + fmt1(w.blk_pct * 100)        + "% BLK" };
}

// ---------------------------------------------------------------------------
// Clutch Player: performance in close late-game situations
// Formula: (clutch_pts/clutch_min)/2.0×0.60 + (clutch_plus_minus/20.0)×0.40
// Falls back to most minutes played when no clutch data is available.
// ---------------------------------------------------------------------------

AwardResult AwardEngine::computeClutch(const std::vector<PlayerProfile>& pool) const {
    bool any_clutch_data = std::any_of(pool.begin(), pool.end(),
                                       [](const PlayerProfile& p){ return p.clutch_min > 0.0; });

    if (!any_clutch_data) {
        auto [idx, score] = selectWinner(pool, [](const PlayerProfile& p){ return p.minutes; });
        (void)score;
        return { AwardType::ClutchPlayer, pool[idx].player_name, pool[idx].team_id,
                 0.0, "Clutch data unavailable" };
    }

    auto [idx, score] = selectWinner(pool, [](const PlayerProfile& p) {
        double pts_per_min = (p.clutch_min > 0.0) ? (p.clutch_pts / p.clutch_min) : 0.0;
        return (pts_per_min / 2.0)          * 0.60
             + (p.clutch_plus_minus / 20.0) * 0.40;
    });
    const auto& w = pool[idx];
    return { AwardType::ClutchPlayer, w.player_name, w.team_id, score,
             w.player_name + " — "
             + fmt1(w.clutch_pts) + " clutch pts, "
             + (w.clutch_plus_minus >= 0 ? "+" : "")
             + fmt1(w.clutch_plus_minus) + " +/-" };
}

// ---------------------------------------------------------------------------
// X-Factor: highly efficient despite low usage — the surprise weapon
// Formula: (ts_pct/usg_pct)×0.70 + (1.0-usg_pct)×0.30
// Guard: players with usg_pct == 0 score 0 to avoid division by zero.
// ---------------------------------------------------------------------------

AwardResult AwardEngine::computeXFactor(const std::vector<PlayerProfile>& pool) const {
    auto [idx, score] = selectWinner(pool, [](const PlayerProfile& p) {
        double ratio = (p.usg_pct > 0.0) ? (p.ts_pct / p.usg_pct) : 0.0;
        return ratio              * 0.70
             + (1.0 - p.usg_pct) * 0.30;
    });
    const auto& w = pool[idx];
    double ratio = (w.usg_pct > 0.0) ? (w.ts_pct / w.usg_pct) : 0.0;
    return { AwardType::XFactor, w.player_name, w.team_id, score,
             w.player_name + " — "
             + fmt1(ratio) + " eff/usage ratio ("
             + fmt0(w.usg_pct * 100) + "% USG)" };
}

} // namespace ss
