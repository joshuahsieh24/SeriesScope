#include "FixtureLoader.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace seriesscope {
std::vector<TeamProfile> FixtureLoader::loadTeamsFromJson(const std::string& filePath) {
    std::ifstream f(filePath);
    nlohmann::json data = nlohmann::json::parse(f);
    return data.get<std::vector<TeamProfile>>();
}
}
