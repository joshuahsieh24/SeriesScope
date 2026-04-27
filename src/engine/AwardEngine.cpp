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
    std::vector<double> scores;
    scores.reserve(pool.size());
    for (const auto& p : pool) {
        double s = p.usg_pct * 0.35
                 + p.ts_pct  * 0.35
                 + (p.off_rating / 130.0) * 0.30;
        scores.push_back(s);
    }

    auto it  = std::max_element(scores.begin(), scores.end());
    auto idx = static_cast<size_t>(it - scores.begin());
    double mn = *std::min_element(scores.begin(), scores.end());
    double mx = *it;

    const auto& winner = pool[idx];
    AwardResult r;
    r.award       = AwardType::SeriesMVP;
    r.player_name = winner.player_name;
    r.team_id     = winner.team_id;
    r.score       = normalize(scores[idx], mn, mx);
    r.rationale   = winner.player_name + " — "
                  + fmt0(winner.usg_pct * 100) + "% USG, "
                  + fmt0(winner.ts_pct  * 100) + "% TS, "
                  + fmt0(winner.off_rating)     + " ORtg";
    return r;
}

// ---------------------------------------------------------------------------
// Best Defender: defensive rating + steal/block/reb rates
// Formula: stl_pct×0.25 + blk_pct×0.25 + dreb_pct×0.20 + ((130-def_rating)/40)×0.30
// Lower def_rating is better; (130-x)/40 converts to a positive signal.
// ---------------------------------------------------------------------------

AwardResult AwardEngine::computeDefender(const std::vector<PlayerProfile>& pool) const {
    std::vector<double> scores;
    scores.reserve(pool.size());
    for (const auto& p : pool) {
        double def_signal = (130.0 - p.def_rating) / 40.0;
        double s = p.stl_pct  * 0.25
                 + p.blk_pct  * 0.25
                 + p.dreb_pct * 0.20
                 + def_signal  * 0.30;
        scores.push_back(s);
    }

    auto it  = std::max_element(scores.begin(), scores.end());
    auto idx = static_cast<size_t>(it - scores.begin());
    double mn = *std::min_element(scores.begin(), scores.end());
    double mx = *it;

    const auto& winner = pool[idx];
    AwardResult r;
    r.award       = AwardType::BestDefender;
    r.player_name = winner.player_name;
    r.team_id     = winner.team_id;
    r.score       = normalize(scores[idx], mn, mx);
    r.rationale   = winner.player_name + " — "
                  + fmt0(winner.def_rating)      + " DRtg, "
                  + fmt1(winner.stl_pct * 100)   + "% STL, "
                  + fmt1(winner.blk_pct * 100)   + "% BLK";
    return r;
}

// ---------------------------------------------------------------------------
// Clutch Player: performance in close late-game situations
// Formula: (clutch_pts/clutch_min)/2.0×0.60 + (clutch_plus_minus/20.0)×0.40
// Guard: if all clutch_min == 0, fall back to winner-by-minutes with a note.
// ---------------------------------------------------------------------------

AwardResult AwardEngine::computeClutch(const std::vector<PlayerProfile>& pool) const {
    bool any_clutch_data = std::any_of(pool.begin(), pool.end(),
                                       [](const PlayerProfile& p){ return p.clutch_min > 0.0; });

    std::vector<double> scores;
    scores.reserve(pool.size());

    if (!any_clutch_data) {
        // Fall back to minutes so we still return a valid name
        for (const auto& p : pool) scores.push_back(p.minutes);
        auto it  = std::max_element(scores.begin(), scores.end());
        auto idx = static_cast<size_t>(it - scores.begin());
        AwardResult r;
        r.award       = AwardType::ClutchPlayer;
        r.player_name = pool[idx].player_name;
        r.team_id     = pool[idx].team_id;
        r.score       = 0.0;
        r.rationale   = "Clutch data unavailable";
        return r;
    }

    for (const auto& p : pool) {
        double pts_per_min = (p.clutch_min > 0.0) ? (p.clutch_pts / p.clutch_min) : 0.0;
        double s = (pts_per_min / 2.0)             * 0.60
                 + (p.clutch_plus_minus / 20.0)    * 0.40;
        scores.push_back(s);
    }

    auto it  = std::max_element(scores.begin(), scores.end());
    auto idx = static_cast<size_t>(it - scores.begin());
    double mn = *std::min_element(scores.begin(), scores.end());
    double mx = *it;

    const auto& winner = pool[idx];
    AwardResult r;
    r.award       = AwardType::ClutchPlayer;
    r.player_name = winner.player_name;
    r.team_id     = winner.team_id;
    r.score       = normalize(scores[idx], mn, mx);
    r.rationale   = winner.player_name + " — "
                  + fmt1(winner.clutch_pts)       + " clutch pts, "
                  + (winner.clutch_plus_minus >= 0 ? "+" : "")
                  + fmt1(winner.clutch_plus_minus) + " +/-";
    return r;
}

// ---------------------------------------------------------------------------
// X-Factor: highly efficient despite low usage — the surprise weapon
// Formula: (ts_pct/usg_pct)×0.70 + (1.0-usg_pct)×0.30
// Guard: skip players with usg_pct == 0 to avoid division by zero.
// ---------------------------------------------------------------------------

AwardResult AwardEngine::computeXFactor(const std::vector<PlayerProfile>& pool) const {
    std::vector<double> scores;
    scores.reserve(pool.size());
    for (const auto& p : pool) {
        double ratio = (p.usg_pct > 0.0) ? (p.ts_pct / p.usg_pct) : 0.0;
        double s = ratio         * 0.70
                 + (1.0 - p.usg_pct) * 0.30;
        scores.push_back(s);
    }

    auto it  = std::max_element(scores.begin(), scores.end());
    auto idx = static_cast<size_t>(it - scores.begin());
    double mn = *std::min_element(scores.begin(), scores.end());
    double mx = *it;

    const auto& winner = pool[idx];
    double ratio = (winner.usg_pct > 0.0) ? (winner.ts_pct / winner.usg_pct) : 0.0;
    AwardResult r;
    r.award       = AwardType::XFactor;
    r.player_name = winner.player_name;
    r.team_id     = winner.team_id;
    r.score       = normalize(scores[idx], mn, mx);
    r.rationale   = winner.player_name + " — "
                  + fmt1(ratio) + " eff/usage ratio ("
                  + fmt0(winner.usg_pct * 100) + "% USG)";
    return r;
}

} // namespace ss
