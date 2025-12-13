#pragma once
#include <string>
#include <sstream>
#include <iomanip>

namespace seriesscope {
class StringUtils {
public:
    static std::string formatPercentage(double value) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << (value * 100.0) << "%";
        return ss.str();
    }

    static std::string formatWinLoss(int wins, int losses) {
        return std::to_string(wins) + "-" + std::to_string(losses);
    }
};
}
