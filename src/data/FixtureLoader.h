#pragma once
#include <string>
#include <vector>
#include "../models/TeamProfile.h"

namespace seriesscope {
class FixtureLoader {
public:
    static std::vector<TeamProfile> loadTeamsFromJson(const std::string& filePath);
};
}
