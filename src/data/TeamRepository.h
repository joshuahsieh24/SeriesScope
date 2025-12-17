#pragma once
#include "DatabaseManager.h"
#include "../models/TeamProfile.h"
#include <vector>

namespace seriesscope {
class TeamRepository {
public:
    TeamRepository(DatabaseManager& db) : db(db) {}
    void upsertTeam(const TeamProfile& team);
    std::vector<TeamProfile> getAllTeams();
    TeamProfile getTeamById(const std::string& id);
private:
    DatabaseManager& db;
};
}
