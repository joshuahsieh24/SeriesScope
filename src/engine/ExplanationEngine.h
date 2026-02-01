#pragma once
#include "../models/TeamProfile.h"
#include "../models/FactorExplanation.h"
#include <vector>

namespace seriesscope {
class ExplanationEngine {
public:
    static AnalysisReport analyze(const TeamProfile& home, const TeamProfile& away, double homeWinPct, double awayWinPct);
};
}
