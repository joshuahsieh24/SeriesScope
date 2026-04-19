#pragma once
#include "models/TeamProfile.h"
#include "models/ScenarioConfig.h"
#include "models/SimulationResult.h"
#include "models/FactorExplanation.h"

namespace ss {

// Produces human-readable factor explanations and a primary narrative from
// the analytical inputs.  No Qt dependency — output is std::string-based.
//
// Design: magnitude for each factor equals (pts_contribution / LOGISTIC_SCALE),
// clamped to [0, 1], so it reflects the factor's actual share of the logit —
// not an arbitrary scale.  Factors are sorted by magnitude descending so the
// UI renders the most decisive reasons first.
class ExplanationEngine {
public:
    AnalysisReport analyze(
        const TeamProfile&       a,
        const TeamProfile&       b,
        const ScenarioConfig&    cfg,
        const AggregatedResults& results) const;

    std::string scenarioDeltaNote(
        const std::string&        abbrev_a,
        const std::string&        abbrev_b,
        const ScenarioComparison& cmp) const;

private:
    void addEfficiencyFactor  (std::vector<FactorExplanation>&,
                               const TeamProfile& a, const TeamProfile& b,
                               const ScenarioConfig&) const;
    void addWinPctFactor      (std::vector<FactorExplanation>&,
                               const TeamProfile& a, const TeamProfile& b) const;
    void addHomeCourt         (std::vector<FactorExplanation>&,
                               const TeamProfile& a, const TeamProfile& b,
                               const ScenarioConfig&) const;
    void addRecentForm        (std::vector<FactorExplanation>&,
                               const TeamProfile& a, const TeamProfile& b,
                               const ScenarioConfig&) const;
    void addStarAvailability  (std::vector<FactorExplanation>&,
                               const TeamProfile& a, const TeamProfile& b,
                               const ScenarioConfig&) const;
    void addVolatility        (std::vector<FactorExplanation>&,
                               const TeamProfile& a, const TeamProfile& b,
                               const ScenarioConfig&) const;
    void addDepth             (std::vector<FactorExplanation>&,
                               const TeamProfile& a, const TeamProfile& b) const;

    std::string buildNarrative(
        const TeamProfile&       a,
        const TeamProfile&       b,
        const AggregatedResults& results) const;

    static double ptsToMagnitude(double pts);
};

} // namespace ss
