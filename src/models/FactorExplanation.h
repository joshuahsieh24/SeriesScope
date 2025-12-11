#pragma once
#include <string>
#include <vector>

namespace seriesscope {
struct FactorExplanation {
    std::string factor_name;
    double impact_magnitude;
    std::string description;
};

struct AnalysisReport {
    std::string summary;
    std::vector<FactorExplanation> key_factors;
    double confidence_score;
};
}
