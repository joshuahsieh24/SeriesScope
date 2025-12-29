#include "ExplanationEngine.h"
#include <algorithm>

namespace seriesscope {
AnalysisReport ExplanationEngine::analyze(const TeamProfile& home, const TeamProfile& away) {
    AnalysisReport report;
    report.summary = home.name + " vs " + away.name;
    
    std::vector<FactorExplanation> factors;
    factors.push_back({"Net Rating", std::abs(home.net_rating - away.net_rating), "Overall efficiency margin."});
    factors.push_back({"Offensive Prowess", std::abs(home.off_rating - away.off_rating), "Scoring capability difference."});
    
    std::sort(factors.begin(), factors.end(), [](const auto& a, const auto& b) {
        return a.impact_magnitude > b.impact_magnitude;
    });
    
    report.key_factors = factors;
    report.confidence_score = 0.85;
    return report;
}
}
