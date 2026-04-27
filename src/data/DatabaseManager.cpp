#include "DatabaseManager.h"
#include <stdexcept>
#include <iostream>

namespace seriesscope {
DatabaseManager::DatabaseManager(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open DB");
    }
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
}

DatabaseManager::~DatabaseManager() {
    if (db) sqlite3_close(db);
}

void DatabaseManager::createSchema() {
    const char* sql = "CREATE TABLE IF NOT EXISTS teams ("
                      "id TEXT PRIMARY KEY, name TEXT, abbreviation TEXT, "
                      "off_rating REAL, def_rating REAL, net_rating REAL, "
                      "efg_pct REAL, tov_pct REAL, orb_pct REAL, ft_rate REAL, primary_color TEXT, "
                      "wins INTEGER DEFAULT 0, losses INTEGER DEFAULT 0, recent_form REAL DEFAULT 0.0, "
                      "star_impact REAL DEFAULT 1.0, volatility REAL DEFAULT 0.2, depth_rating REAL DEFAULT 0.5);";
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);

    // Column order must match PlayerRepository::fromStatement binding indices (0–18).
    const char* player_sql =
        "CREATE TABLE IF NOT EXISTS players ("
        "player_id TEXT PRIMARY KEY, "       // 0
        "player_name TEXT NOT NULL, "        // 1
        "team_id TEXT NOT NULL, "            // 2
        "minutes REAL DEFAULT 0.0, "         // 3
        "pts REAL DEFAULT 0.0, "             // 4
        "stl REAL DEFAULT 0.0, "             // 5
        "blk REAL DEFAULT 0.0, "             // 6
        "gp INTEGER DEFAULT 0, "             // 7
        "usg_pct REAL DEFAULT 0.0, "         // 8
        "off_rating REAL DEFAULT 0.0, "      // 9
        "def_rating REAL DEFAULT 0.0, "      // 10
        "ts_pct REAL DEFAULT 0.0, "          // 11
        "ast_pct REAL DEFAULT 0.0, "         // 12
        "dreb_pct REAL DEFAULT 0.0, "        // 13
        "stl_pct REAL DEFAULT 0.0, "         // 14
        "blk_pct REAL DEFAULT 0.0, "         // 15
        "clutch_min REAL DEFAULT 0.0, "      // 16
        "clutch_pts REAL DEFAULT 0.0, "      // 17
        "clutch_plus_minus REAL DEFAULT 0.0" // 18
        ");";
    sqlite3_exec(db, player_sql, nullptr, nullptr, nullptr);
}
}
