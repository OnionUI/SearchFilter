#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <sqlite3/sqlite3.h>
#include <dirent.h>

using std::string;
using std::stringstream;
using std::map;

#define ROMS_PATH "/mnt/SDCARD/Roms"
#define ROM_PATH(name) ROMS_PATH "/" + name
#define CACHE_PATH(name) ROM_PATH(name) + "/" + name + "_cache2.db"

int db_filterEntries(string db_path, string name, string keyword = "")
{
    int count = 0;
    sqlite3* db;
    sqlite3_stmt* stmt;

    string table = name + "_roms";

    stringstream query, subquery;
    bool first = true;
    char *ptr;
    char* str = (char*)keyword.c_str();

    subquery << sqlite3_mprintf("SELECT id FROM %Q WHERE", table.c_str());
    // Split the keywords at every ' ' (space)
    ptr = strtok(str, " ");
    while (ptr) {
        if (!first)
            subquery << " AND";
        subquery << sqlite3_mprintf(" disp LIKE '%%%q%%'", ptr);
        ptr = strtok(NULL, " ");
        first = false;
    }
    
    // Insert the subquery into the filter query
    query << sqlite3_mprintf("DELETE FROM %Q WHERE id NOT IN (%s);", table.c_str(), subquery.str().c_str());
    subquery << ";";

    // Open the database file
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "ERROR: can't open database: " << sqlite3_errmsg(db) << " [" << db_path << "]" << std::endl;
        sqlite3_close(db);
        return -1;
    }

    int ret_code;

    if (keyword.length() > 0) {
        // Prepare filter query
        if(sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            std::cerr << "ERROR: while compiling sql: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            sqlite3_finalize(stmt);
            return -1;
        }

        // Execute query
        int ret_code = sqlite3_step(stmt);
    }

    // Prepare row count query
    if(sqlite3_prepare_v2(db, sqlite3_mprintf("SELECT count(*) FROM %Q;", table.c_str()), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "ERROR: while compiling sql: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        sqlite3_finalize(stmt);
        return -1;
    }

    // Execute row count query
    while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    if (ret_code != SQLITE_DONE) {
        std::cerr << "ERROR: while performing sql: " << sqlite3_errmsg(db) << std::endl;
        std::cerr << "ret_code = " << std::to_string(ret_code) << std::endl;
        return -1;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return count;
}

void db_clearAll()
{
    subdirForEach(ROMS_PATH, [](string name) {
        string db_path = CACHE_PATH(name);
        string backup_path = db_path + ".backup";

        // If cache does not exist, remove backup and exit
        if (!exists(db_path)) {
            if (exists(backup_path))
                remove(backup_path.c_str());
            return -1;
        }

        // If no backup is found, do nothing
        if (!exists(backup_path) || db_filterEntries(backup_path, name) <= 0)
            return -1;
        
        // Restore the cache from backup
        remove(db_path.c_str());
        rename(backup_path.c_str(), db_path.c_str());

        return -1;
    });
}

map<string, int> db_filterAll(string keyword)
{
    db_clearAll();

    return subdirForEach(ROMS_PATH, [keyword](string name) {
        string rom_path = ROM_PATH(name);
        string db_path = CACHE_PATH(name);
        string backup_path = db_path + ".backup";

        // If cache isn't found, we can't filter it
        if (!exists(db_path))
            return dirEmpty(rom_path) ? -2 : -1;

        // Create backup of cache (if not empty)
        if (!exists(backup_path) && db_filterEntries(db_path, name) > 0)
            copyFile(db_path, backup_path);

        // Filter the cache db and return count
        return db_filterEntries(db_path, name, keyword);
    });
}
