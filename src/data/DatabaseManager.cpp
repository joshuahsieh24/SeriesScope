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
                      "efg_pct REAL, tov_pct REAL, orb_pct REAL, ft_rate REAL, primary_color TEXT);";
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
}
}
