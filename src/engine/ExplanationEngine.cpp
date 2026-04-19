#include "engine/ExplanationEngine.h"
#include "engine/GameProbabilityModel.h"
#include "utils/StringUtils.h"
#include <algorithm>
#include <sstream>
#include <cmath>

namespace ss {

using util::rating;
using util::pct;
using util::fixed1;
using util::normalizeImpact;

// static
double ExplanationEngine::ptsToMagnitude(double pts) {
    return std::clamp(std::abs(pts) / GameProbabilityModel::LOGISTIC_SCALE, 0.0, 1.0);
}

// ── Factor builders ──────────────────────────────────────────────────────────

void ExplanationEngine::addEfficiencyFactor(
    std::vector<FactorExplanation>& factors,
    const TeamProfile& a, const TeamProfile& b,
    const ScenarioConfig& cfg) const
{
    double net_a = a.net_rating + cfg.team_a_off_adj - cfg.team_a_def_adj;
    double net_b = b.net_rating + cfg.team_b_off_adj - cfg.team_b_def_adj;
    double diff  = net_a - net_b;
    if (std::abs(diff) < 0.5) return;

    double pts = diff * GameProbabilityModel::PACE_SCALE;

    FactorExplanation f;
    f.title     = "Efficiency Rating";
    f.magnitude = ptsToMagnitude(pts);

    if (diff > 0) {
        f.beneficiary = FactorExplanation::Beneficiary::TeamA;
        f.detail = a.abbreviation + " net rtg " + rating(a.net_rating)
                 + " vs " + b.abbreviation + " " + rating(b.net_rating)
                 + " — " + fixed1(diff) + "-pt opponent-adjusted edge";
    } else {
        f.beneficiary = FactorExplanation::Beneficiary::TeamB;
        f.detail = b.abbreviation + " net rtg " + rating(b.net_rating)
                 + " vs " + a.abbreviation + " " + rating(a.net_rating)
                 + " — " + fixed1(-diff) + "-pt opponent-adjusted edge";
    }
    factors.push_back(f);
}

void ExplanationEngine::addWinPctFactor(
    std::vector<FactorExplanation>& factors,
    const TeamProfile& a, const TeamProfile& b) const
{
    double wpc_a = (a.wins + 0.5) / (a.wins + a.losses + 1.0);
    double wpc_b = (b.wins + 0.5) / (b.wins + b.losses + 1.0);
    double diff  = wpc_a - wpc_b;
    if (std::abs(diff) < 0.03) return;

    double pts = diff * GameProbabilityModel::WIN_PCT_SCALE;

    FactorExplanation f;
    f.title     = "Win Percentage";
    f.magnitude = ptsToMagnitude(pts);

    auto record = [](const TeamProfile& t) {
        return std::to_string(t.wins) + "-" + std::to_string(t.losses);
    };

    if (diff > 0) {
        f.beneficiary = FactorExplanation::Beneficiary::TeamA;
        f.detail = a.abbreviation + " " + record(a) + " vs "
                 + b.abbreviation + " " + record(b)
                 + " — execution / clutch edge";
    } else {
        f.beneficiary = FactorExplanation::Beneficiary::TeamB;
        f.detail = b.abbreviation + " " + record(b) + " vs "
                 + a.abbreviation + " " + record(a)
                 + " — execution / clutch edge";
    }
    factors.push_back(f);
}

void ExplanationEngine::addHomeCourt(
    std::vector<FactorExplanation>& factors,
    const TeamProfile& a, const TeamProfile& b,
    const ScenarioConfig& cfg) const
{
    FactorExplanation f;
    f.title     = "Home Court";
    f.magnitude = ptsToMagnitude(GameProbabilityModel::HOME_COURT_PTS);

    if (cfg.team_a_has_home_court) {
        f.beneficiary = FactorExplanation::Beneficiary::TeamA;
        f.detail = a.abbreviation + " holds home-court advantage"
                   " (Games 1, 2, 5, 7) — ~3.2 pt swing per game";
    } else {
        f.beneficiary = FactorExplanation::Beneficiary::TeamB;
        f.detail = b.abbreviation + " holds home-court advantage"
                   " (Games 1, 2, 5, 7) — ~3.2 pt swing per game";
    }
    factors.push_back(f);
}

void ExplanationEngine::addRecentForm(
    std::vector<FactorExplanation>& factors,
    const TeamProfile& a, const TeamProfile& b,
    const ScenarioConfig& cfg) const
{
    double form_a = a.recent_form + cfg.team_a_form_adj;
    double form_b = b.recent_form + cfg.team_b_form_adj;
    double diff   = form_a - form_b;
    if (std::abs(diff) < 0.15) return;

    double pts = diff * GameProbabilityModel::FORM_SCALE;

    FactorExplanation f;
    f.title     = "Recent Form";
    f.magnitude = ptsToMagnitude(pts);

    auto formLabel = [](double v) -> std::string {
        if (v >  0.5) return "excellent";
        if (v >  0.1) return "solid";
        if (v > -0.1) return "average";
        if (v > -0.5) return "below average";
        return "poor";
    };

    if (diff > 0) {
        f.beneficiary = FactorExplanation::Beneficiary::TeamA;
        f.detail = a.abbreviation + " in " + formLabel(form_a) + " form, "
                 + b.abbreviation + " in " + formLabel(form_b) + " form"
                 + " (last-10 momentum)";
    } else {
        f.beneficiary = FactorExplanation::Beneficiary::TeamB;
        f.detail = b.abbreviation + " in " + formLabel(form_b) + " form, "
                 + a.abbreviation + " in " + formLabel(form_a) + " form"
                 + " (last-10 momentum)";
    }
    factors.push_back(f);
}

void ExplanationEngine::addStarAvailability(
    std::vector<FactorExplanation>& factors,
    const TeamProfile& a, const TeamProfile& b,
    const ScenarioConfig& cfg) const
{
    bool a_adjusted = cfg.team_a_star_avail < 0.95;
    bool b_adjusted = cfg.team_b_star_avail < 0.95;
    if (!a_adjusted && !b_adjusted) return;

    double penalty_a = (1.0 - cfg.team_a_star_avail) * a.star_impact
                     * GameProbabilityModel::STAR_VALUE_PTS;
    double penalty_b = (1.0 - cfg.team_b_star_avail) * b.star_impact
                     * GameProbabilityModel::STAR_VALUE_PTS;
    double net_pts   = penalty_b - penalty_a;

    FactorExplanation f;
    f.title     = "Star Availability";
    f.magnitude = ptsToMagnitude(penalty_a + penalty_b);

    auto impactLabel = [](double avail) -> std::string {
        if (avail < 0.25) return "severely limited";
        if (avail < 0.60) return "reduced";
        if (avail < 0.85) return "slightly limited";
        return "fully available";
    };

    std::ostringstream detail;
    if (a_adjusted && b_adjusted) {
        f.beneficiary = net_pts >= 0
            ? FactorExplanation::Beneficiary::TeamA
            : FactorExplanation::Beneficiary::TeamB;
        detail << a.abbreviation << " stars " << impactLabel(cfg.team_a_star_avail)
               << "; " << b.abbreviation << " stars " << impactLabel(cfg.team_b_star_avail);
    } else if (a_adjusted) {
        f.beneficiary = FactorExplanation::Beneficiary::TeamB;
        detail << a.abbreviation << " stars " << impactLabel(cfg.team_a_star_avail)
               << " — advantage " << b.abbreviation
               << " (" << fixed1(penalty_a) << "-pt penalty)";
    } else {
        f.beneficiary = FactorExplanation::Beneficiary::TeamA;
        detail << b.abbreviation << " stars " << impactLabel(cfg.team_b_star_avail)
               << " — advantage " << a.abbreviation
               << " (" << fixed1(penalty_b) << "-pt penalty)";
    }

    f.detail = detail.str();
    factors.push_back(f);
}

void ExplanationEngine::addVolatility(
    std::vector<FactorExplanation>& factors,
    const TeamProfile& a, const TeamProfile& b,
    const ScenarioConfig& cfg) const
{
    double combined = (a.volatility + b.volatility) / 2.0 * cfg.volatility_multiplier;
    if (combined < 0.35) return;

    FactorExplanation f;
    f.title       = "Series Volatility";
    f.beneficiary = FactorExplanation::Beneficiary::Neither;
    f.magnitude   = std::min(combined * 0.4, 0.30);

    if (combined > 0.65)
        f.detail = "High combined volatility — expect a competitive, unpredictable series";
    else
        f.detail = "Moderate variance; either team could run off a winning stretch";

    factors.push_back(f);
}

void ExplanationEngine::addDepth(
    std::vector<FactorExplanation>& factors,
    const TeamProfile& a, const TeamProfile& b) const
{
    double diff = a.depth_rating - b.depth_rating;
    if (std::abs(diff) < 0.15) return;

    double pts = diff * 2.0;
    FactorExplanation f;
    f.title     = "Bench Depth";
    f.magnitude = std::min(ptsToMagnitude(pts), 0.25);

    if (diff > 0) {
        f.beneficiary = FactorExplanation::Beneficiary::TeamA;
        f.detail = a.abbreviation + " has the deeper bench"
                   " — advantage in longer, physical series";
    } else {
        f.beneficiary = FactorExplanation::Beneficiary::TeamB;
        f.detail = b.abbreviation + " has the deeper bench"
                   " — advantage in longer, physical series";
    }
    factors.push_back(f);
}

// ── Narrative builder ────────────────────────────────────────────────────────

std::string ExplanationEngine::buildNarrative(
    const TeamProfile& a, const TeamProfile& b,
    const AggregatedResults& r) const
{
    std::ostringstream s;
    bool   a_wins  = r.most_likely_winner_is_a();
    double fav_pct = a_wins ? r.team_a_win_pct() : r.team_b_win_pct();
    int    len     = r.most_likely_length();
    const auto& fav = a_wins ? a : b;
    const auto& dog = a_wins ? b : a;

    s << fav.name << " enter as the ";
    if (fav_pct > 0.80)      s << "strong favorite";
    else if (fav_pct > 0.65) s << "clear favorite";
    else if (fav_pct > 0.55) s << "slight favorite";
    else                     s << "marginal favorite";

    s << " (" << pct(fav_pct, 0) << ") over " << dog.name << ".";
    s << "  Most likely outcome: " << fav.abbreviation << " in " << len << ".";
    return s.str();
}

// ── Public API ────────────────────────────────────────────────────────────────

AnalysisReport ExplanationEngine::analyze(
    const TeamProfile&       a,
    const TeamProfile&       b,
    const ScenarioConfig&    cfg,
    const AggregatedResults& results) const
{
    AnalysisReport report;

    addEfficiencyFactor (report.factors, a, b, cfg);
    addWinPctFactor     (report.factors, a, b);
    addHomeCourt        (report.factors, a, b, cfg);
    addRecentForm       (report.factors, a, b, cfg);
    addStarAvailability (report.factors, a, b, cfg);
    addDepth            (report.factors, a, b);
    addVolatility       (report.factors, a, b, cfg);

    std::sort(report.factors.begin(), report.factors.end(),
        [](const FactorExplanation& x, const FactorExplanation& y){
            return x.magnitude > y.magnitude;
        });

    report.primary_narrative = buildNarrative(a, b, results);
    return report;
}

std::string ExplanationEngine::scenarioDeltaNote(
    const std::string&        abbrev_a,
    const std::string&        abbrev_b,
    const ScenarioComparison& cmp) const
{
    double delta = cmp.delta_a() * 100.0;
    std::ostringstream s;

    if (std::abs(delta) < 0.5) {
        s << "Scenario adjustment had negligible impact on the series projection.";
    } else if (delta > 0) {
        s << "Adjustment favors " << abbrev_a << " by +" << fixed1(delta) << "pp"
          << " (" << pct(cmp.baseline.team_a_win_pct(), 0)
          << " \xe2\x86\x92 " << pct(cmp.adjusted.team_a_win_pct(), 0) << ")";
    } else {
        s << "Adjustment favors " << abbrev_b << " by +" << fixed1(-delta) << "pp"
          << " (" << pct(cmp.baseline.team_b_win_pct(), 0)
          << " \xe2\x86\x92 " << pct(cmp.adjusted.team_b_win_pct(), 0) << ")";
    }
    return s.str();
}

} // namespace ss
