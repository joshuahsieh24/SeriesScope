#include "ExplanationEngine.h"
#include <algorithm>

namespace seriesscope {
AnalysisReport ExplanationEngine::analyze(const TeamProfile& home, const TeamProfile& away, double hWP, double aWP) {
    AnalysisReport report;
    report.summary = home.name + " vs " + away.name;
    std::vector<FactorExplanation> factors;
    factors.push_back({"Win Percentage", std::abs(hWP - aWP), "Historical success factor."});
    factors.push_back({"Net Rating", std::abs(home.net_rating - away.net_rating), "Overall efficiency margin."});
    std::sort(factors.begin(), factors.end(), [](const auto& a, const auto& b) {
        return a.impact_magnitude > b.impact_magnitude;
    });
    report.key_factors = factors;
    report.confidence_score = 0.92;
    return report;
}
}
