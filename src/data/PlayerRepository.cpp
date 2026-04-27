#include "PlayerRepository.h"
#include <sqlite3.h>

namespace seriesscope {

// Column indices 0–18 must stay in sync with the players table DDL in DatabaseManager.cpp.
PlayerProfile PlayerRepository::fromStatement(sqlite3_stmt* stmt) {
    PlayerProfile p;
    p.player_id   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    p.player_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    p.team_id     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    p.minutes     = sqlite3_column_double(stmt, 3);
    p.pts         = sqlite3_column_double(stmt, 4);
    p.stl         = sqlite3_column_double(stmt, 5);
    p.blk         = sqlite3_column_double(stmt, 6);
    p.gp          = sqlite3_column_int   (stmt, 7);
    p.usg_pct     = sqlite3_column_double(stmt, 8);
    p.off_rating  = sqlite3_column_double(stmt, 9);
    p.def_rating  = sqlite3_column_double(stmt, 10);
    p.ts_pct      = sqlite3_column_double(stmt, 11);
    p.ast_pct     = sqlite3_column_double(stmt, 12);
    p.dreb_pct    = sqlite3_column_double(stmt, 13);
    p.stl_pct     = sqlite3_column_double(stmt, 14);
    p.blk_pct     = sqlite3_column_double(stmt, 15);
    p.clutch_min          = sqlite3_column_double(stmt, 16);
    p.clutch_pts          = sqlite3_column_double(stmt, 17);
    p.clutch_plus_minus   = sqlite3_column_double(stmt, 18);
    return p;
}

void PlayerRepository::upsertPlayer(const PlayerProfile& p) {
    const char* sql =
        "INSERT OR REPLACE INTO players "
        "(player_id, player_name, team_id, minutes, pts, stl, blk, gp, "
        "usg_pct, off_rating, def_rating, ts_pct, ast_pct, dreb_pct, stl_pct, blk_pct, "
        "clutch_min, clutch_pts, clutch_plus_minus) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text  (stmt,  1, p.player_id.c_str(),   -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt,  2, p.player_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt,  3, p.team_id.c_str(),     -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt,  4, p.minutes);
    sqlite3_bind_double(stmt,  5, p.pts);
    sqlite3_bind_double(stmt,  6, p.stl);
    sqlite3_bind_double(stmt,  7, p.blk);
    sqlite3_bind_int   (stmt,  8, p.gp);
    sqlite3_bind_double(stmt,  9, p.usg_pct);
    sqlite3_bind_double(stmt, 10, p.off_rating);
    sqlite3_bind_double(stmt, 11, p.def_rating);
    sqlite3_bind_double(stmt, 12, p.ts_pct);
    sqlite3_bind_double(stmt, 13, p.ast_pct);
    sqlite3_bind_double(stmt, 14, p.dreb_pct);
    sqlite3_bind_double(stmt, 15, p.stl_pct);
    sqlite3_bind_double(stmt, 16, p.blk_pct);
    sqlite3_bind_double(stmt, 17, p.clutch_min);
    sqlite3_bind_double(stmt, 18, p.clutch_pts);
    sqlite3_bind_double(stmt, 19, p.clutch_plus_minus);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<PlayerProfile> PlayerRepository::getPlayersByTeam(const std::string& team_id) const {
    std::vector<PlayerProfile> players;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_.handle(),
        "SELECT * FROM players WHERE team_id = ? ORDER BY minutes DESC LIMIT 5;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, team_id.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        players.push_back(fromStatement(stmt));
    sqlite3_finalize(stmt);
    return players;
}

std::vector<PlayerProfile> PlayerRepository::getPlayersForMatchup(
    const std::string& team_a_id,
    const std::string& team_b_id) const
{
    auto a = getPlayersByTeam(team_a_id);
    auto b = getPlayersByTeam(team_b_id);
    a.insert(a.end(), b.begin(), b.end());
    return a;  // up to 10 players
}

} // namespace seriesscope
