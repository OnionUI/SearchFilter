#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <functional>
#include <algorithm>
#include <map>
#include <sqlite3/sqlite3.h>
#include <dirent.h>

using std::string;
using std::stringstream;
using std::function;
using std::map;

#define ROMS_PATH "/mnt/SDCARD/Roms"
#define IGNORE_HIDDEN_FILES 1

int db_filterEntries(string name, string keyword)
{
    int count = 0;
    sqlite3* db;
    sqlite3_stmt* stmt;

    string filename = ROMS_PATH "/" + name + "/" + name + "_cache2.db";
    string table = name + "_roms";

    stringstream query, subquery;
    bool first = true;
    char *ptr;
    char* str = (char*)keyword.c_str();
    ptr = strtok(str, " ");
    subquery << sqlite3_mprintf("SELECT id FROM %Q WHERE", table.c_str());
    while (ptr) {
        if (!first)
            subquery << " AND";
        subquery << sqlite3_mprintf(" disp LIKE '%%%q%%'", ptr);
        ptr = strtok(NULL, " ");
        first = false;
    }
    query << sqlite3_mprintf("DELETE FROM %Q WHERE id NOT IN (%s);", table.c_str(), subquery.str().c_str());
    subquery << ";";

    //get link to database object
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "ERROR: can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }

    // compile sql statement to binary
    if(sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "ERROR: while compiling sql: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        sqlite3_finalize(stmt);
        return -1;
    }

    // execute sql statement, and step through rows
    int ret_code = sqlite3_step(stmt);

    // compile sql statement to binary
    if(sqlite3_prepare_v2(db, sqlite3_mprintf("SELECT count(*) FROM %Q;", table.c_str()), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "ERROR: while compiling sql: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        sqlite3_finalize(stmt);
        return -1;
    }

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

map<string, int> db_applyToAll(function<int(string)> callback) {
    map<string, int> counts = map<string, int>();
    DIR* dirFile = opendir(ROMS_PATH);

    if (!dirFile)
        return counts;

    struct dirent* item;
    errno = 0;

    while ((item = readdir(dirFile)) != NULL) {
        string name(item->d_name);

        if (item->d_type != DT_DIR) continue;
        if (name == "." || name == "..") continue;

        // in linux hidden files all start with '.'
        if (IGNORE_HIDDEN_FILES && (name[0] == '.')) continue;

        counts[name] = callback(name);
    }

    closedir(dirFile);
    return counts;
}

void db_clearAll()
{
    db_applyToAll([](string name) {
        string db_path = ROMS_PATH "/" + name + "/" + name + "_cache2.db";
        string backup_path = db_path + ".backup";
        if (!exists(backup_path))
            return -1;
        remove(db_path.c_str());
        rename(backup_path.c_str(), db_path.c_str());
        return -1;
    });
}

map<string, int> db_filterAll(string keyword)
{
    db_clearAll();

    return db_applyToAll([keyword](string name) {
        string db_path = ROMS_PATH "/" + name + "/" + name + "_cache2.db";
        string backup_path = db_path + ".backup";
        if (!exists(backup_path))
            copyFile(db_path, backup_path);
        return db_filterEntries(name, keyword);
    });
}
