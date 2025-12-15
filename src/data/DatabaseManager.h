#pragma once
#include <string>
#include <sqlite3.h>
#include <memory>

namespace seriesscope {
class DatabaseManager {
public:
    DatabaseManager(const std::string& dbPath);
    ~DatabaseManager();
    sqlite3* handle() const { return db; }
    void createSchema();
private:
    sqlite3* db = nullptr;
};
}
