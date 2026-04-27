#pragma once

#include "DatabaseManager.h"
#include "../models/PlayerProfile.h"
#include <vector>
#include <string>

namespace seriesscope {

class PlayerRepository {
public:
    explicit PlayerRepository(DatabaseManager& db) : db_(db) {}

    void upsertPlayer(const PlayerProfile& p);

    // Returns up to 5 players for a single team, ordered by minutes DESC.
    std::vector<PlayerProfile> getPlayersByTeam(const std::string& team_id) const;

    // Returns up to 10 players (5 per team) for a matchup.
    // Column order in SELECT * must match fromStatement indices (see DatabaseManager.cpp DDL).
    std::vector<PlayerProfile> getPlayersForMatchup(
        const std::string& team_a_id,
        const std::string& team_b_id) const;

private:
    DatabaseManager& db_;

    // Reads a single row from a prepared SELECT * statement into a PlayerProfile.
    // Column indices 0–18 must match the players table DDL in DatabaseManager.cpp.
    static PlayerProfile fromStatement(sqlite3_stmt* stmt);
};

} // namespace seriesscope
