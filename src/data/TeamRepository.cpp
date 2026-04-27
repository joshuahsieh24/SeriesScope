#include "TeamRepository.h"
#include <sqlite3.h>
#include <stdexcept>

namespace seriesscope {
void TeamRepository::upsertTeam(const TeamProfile& t) {
    const char* sql = "INSERT OR REPLACE INTO teams "
                      "(id, name, abbreviation, off_rating, def_rating, net_rating, efg_pct, tov_pct, orb_pct, ft_rate, primary_color, wins, losses, recent_form, star_impact, volatility, depth_rating) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, t.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, t.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, t.abbreviation.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, t.off_rating);
    sqlite3_bind_double(stmt, 5, t.def_rating);
    sqlite3_bind_double(stmt, 6, t.net_rating);
    sqlite3_bind_double(stmt, 7, t.eFG_pct);
    sqlite3_bind_double(stmt, 8, t.TOV_pct);
    sqlite3_bind_double(stmt, 9, t.ORB_pct);
    sqlite3_bind_double(stmt, 10, t.FT_rate);
    sqlite3_bind_text(stmt, 11, t.primary_color.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 12, t.wins);
    sqlite3_bind_int(stmt, 13, t.losses);
    sqlite3_bind_double(stmt, 14, t.recent_form);
    sqlite3_bind_double(stmt, 15, t.star_impact);
    sqlite3_bind_double(stmt, 16, t.volatility);
    sqlite3_bind_double(stmt, 17, t.depth_rating);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<TeamProfile> TeamRepository::getAllTeams() {
    std::vector<TeamProfile> teams;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db.handle(), "SELECT * FROM teams;", -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TeamProfile t;
        t.id = (const char*)sqlite3_column_text(stmt, 0);
        t.name = (const char*)sqlite3_column_text(stmt, 1);
        t.abbreviation = (const char*)sqlite3_column_text(stmt, 2);
        t.off_rating = sqlite3_column_double(stmt, 3);
        t.def_rating = sqlite3_column_double(stmt, 4);
        t.net_rating = sqlite3_column_double(stmt, 5);
        t.eFG_pct = sqlite3_column_double(stmt, 6);
        t.TOV_pct = sqlite3_column_double(stmt, 7);
        t.ORB_pct = sqlite3_column_double(stmt, 8);
        t.FT_rate = sqlite3_column_double(stmt, 9);
        t.primary_color = (const char*)sqlite3_column_text(stmt, 10);
        t.wins = sqlite3_column_int(stmt, 11);
        t.losses = sqlite3_column_int(stmt, 12);
        t.recent_form = sqlite3_column_double(stmt, 13);
        t.star_impact = sqlite3_column_double(stmt, 14);
        t.volatility = sqlite3_column_double(stmt, 15);
        t.depth_rating = sqlite3_column_double(stmt, 16);
        teams.push_back(t);
    }
    sqlite3_finalize(stmt);
    return teams;
}

TeamProfile TeamRepository::getTeamById(const std::string& id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db.handle(), "SELECT * FROM teams WHERE id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    TeamProfile t;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        t.id = (const char*)sqlite3_column_text(stmt, 0);
        t.name = (const char*)sqlite3_column_text(stmt, 1);
        t.abbreviation = (const char*)sqlite3_column_text(stmt, 2);
        t.off_rating = sqlite3_column_double(stmt, 3);
        t.def_rating = sqlite3_column_double(stmt, 4);
        t.net_rating = sqlite3_column_double(stmt, 5);
        t.eFG_pct = sqlite3_column_double(stmt, 6);
        t.TOV_pct = sqlite3_column_double(stmt, 7);
        t.ORB_pct = sqlite3_column_double(stmt, 8);
        t.FT_rate = sqlite3_column_double(stmt, 9);
        t.primary_color = (const char*)sqlite3_column_text(stmt, 10);
        t.wins = sqlite3_column_int(stmt, 11);
        t.losses = sqlite3_column_int(stmt, 12);
        t.recent_form = sqlite3_column_double(stmt, 13);
        t.star_impact = sqlite3_column_double(stmt, 14);
        t.volatility = sqlite3_column_double(stmt, 15);
        t.depth_rating = sqlite3_column_double(stmt, 16);
    }
    sqlite3_finalize(stmt);
    return t;
}
}
