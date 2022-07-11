#if !defined(DB_CACHE_HPP__)
#define DB_CACHE_HPP__

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>
#include <sqlite3/sqlite3.h>

using std::string;
using std::to_string;
using std::stringstream;
using std::map;
using std::vector;

#include "utils.hpp"

#define ROMS_PATH "/mnt/SDCARD/Roms"
#define ROM_PATH(name) ROMS_PATH "/" + name
#define CACHE_NAME(name) name + "_cache2.db"
#define CACHE_PATH(name) ROM_PATH(name) + "/" + CACHE_NAME(name)
#define TABLE_NAME(name) name + "_roms"

struct RomEntry
{
    string disp = "";
    string path = "";
    string imgpath = "";
    int type = 0;
    string ppath = ".";

    RomEntry changePpath(string _ppath) {
        return {.disp = disp, .path = path, .imgpath = imgpath, .type = type, .ppath = _ppath};
    }
};

namespace db::sql {

string search(string table, const string &keyword_str)
{
    string sql = sqlite3_mprintf("SELECT * FROM %Q", table.c_str());
    vector<string> keywords = split(keyword_str, " ");

    bool first = true;
    int count = keywords.size();

    for (int i = 0; i < count; i++) {
        string keyword = trim(keywords[i]);
        if (keyword.length() == 0)
            continue;
        sql += (first ? " WHERE" : " AND");
        sql += sqlite3_mprintf(" disp LIKE '%%%q%%'", keyword.c_str());
        first = false;
    }

    return sql;
}

string subdirs(string table, string search_query)
{
    string sql = 
        "SELECT id, path AS _path FROM %Q WHERE type=1 AND EXISTS ("
            "SELECT id FROM %Q WHERE path LIKE _path || '%%' AND id IN ("
                "SELECT id FROM (%s)))";
    return string(sqlite3_mprintf(sql.c_str(), table.c_str(), table.c_str(), search_query.c_str()));
}

string filter(string table, string search_query)
{
    string sql =
        "DELETE FROM %Q WHERE "
            "type=0 AND id NOT IN (SELECT id FROM (%s)) OR "
            "type=1 AND id NOT IN (SELECT id FROM (%s));";
    string subdir_query = subdirs(table, search_query);
    return string(sqlite3_mprintf(sql.c_str(), table.c_str(), search_query.c_str(), subdir_query.c_str()));
}

string create_roms_table(string name)
{
    string sql =
        "CREATE TABLE %Q ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "disp TEXT NOT NULL,"
            "path TEXT NOT NULL,"
            "imgpath TEXT NOT NULL,"
            "type INTEGER DEFAULT 0,"
            "ppath TEXT NOT NULL)";
    string table = TABLE_NAME(name);
    return string(sqlite3_mprintf(sql.c_str(), table.c_str()));
}

string insert(string name, RomEntry entry)
{
    string sql =
        "INSERT INTO %Q (disp, path, imgpath, type, ppath) "
            "VALUES (%Q, %Q, %Q, %q, %Q);";
    string table = TABLE_NAME(name);
    return string(sqlite3_mprintf(
        sql.c_str(),
        table.c_str(),
        entry.disp.c_str(),
        entry.path.c_str(),
        entry.imgpath.c_str(),
        to_string(entry.type).c_str(),
        entry.ppath.c_str()
    ));
}

string dupChangePpath(string name, string ppath)
{
    string sql =
        "INSERT INTO %Q (disp, path, imgpath, type, ppath) "
        "SELECT disp, path, imgpath, type, %Q FROM %Q WHERE type=0 AND path!='nocache';";
    string table = TABLE_NAME(name);
    return string(sqlite3_mprintf(
        sql.c_str(),
        table.c_str(),
        ppath.c_str(),
        table.c_str()
    ));
}

} // namespace db::sql

namespace db {

bool open(sqlite3** db, string path)
{
    if (sqlite3_open(path.c_str(), db) != SQLITE_OK) {
        std::cerr << "ERROR: can't open database: " << sqlite3_errmsg(*db) << " [" << path << "]" << std::endl;
        sqlite3_close(*db);
        return false;
    }

    return true;
}

bool open_readonly(sqlite3** db, string path)
{
    if (sqlite3_open_v2(path.c_str(), db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        std::cerr << "ERROR: can't open database: " << sqlite3_errmsg(*db) << " [" << path << "]" << std::endl;
        sqlite3_close(*db);
        return false;
    }

    return true;
}

bool prepare(sqlite3* db, sqlite3_stmt** stmt, const char* query)
{
    if(sqlite3_prepare_v2(db, query, -1, stmt, NULL) != SQLITE_OK) {
        std::cerr << "ERROR: while compiling sql: " << sqlite3_errmsg(db) << std::endl;
        std::cerr << "sql: " << query << std::endl;
        sqlite3_finalize(*stmt);
        sqlite3_close(db);
        return false;
    }

    return true;
}
bool prepare(sqlite3* db, sqlite3_stmt** stmt, string query)
{
    return prepare(db, stmt, query.c_str());
}

bool isDone(sqlite3* db, int rc)
{
    if (rc != SQLITE_DONE) {
        std::cerr << "ERROR: while performing sql: " << sqlite3_errmsg(db) << std::endl;
        std::cerr << "Return code: " << to_string(rc) << std::endl;
        return false;
    }

    return true;
}

bool create(string path, string name)
{
    int rc;
    sqlite3* db;
    sqlite3_stmt* stmt;

    // Open the database file
    if (!open(&db, path))
        return false;

    // Create roms table
    if(!prepare(db, &stmt, sql::create_roms_table(name)))
        return false;
    rc = sqlite3_step(stmt);
    
    if (!isDone(db, rc))
        return false;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

bool insertRom(sqlite3* db, string name, RomEntry entry)
{
    int rc;
    sqlite3_stmt* stmt;

    // Create roms table
    if(!prepare(db, &stmt, sql::insert(name, entry)))
        return false;
    rc = sqlite3_step(stmt);
    
    if (!isDone(db, rc))
        return false;

    sqlite3_finalize(stmt);
    return true;
}

bool duplicateResults(sqlite3* db, string name, string ppath)
{
    int rc;
    sqlite3_stmt* stmt;

    // Create roms table
    if(!prepare(db, &stmt, sql::dupChangePpath(name, ppath)))
        return false;
    rc = sqlite3_step(stmt);
    
    if (!isDone(db, rc))
        return false;

    sqlite3_finalize(stmt);
    return true;
}

vector<RomEntry> searchEntries(string name, string keyword)
{
    int rc;
    vector<RomEntry> entries;

    if (keyword.length() == 0)
        return entries;

    string path = CACHE_PATH(name);
    string table = TABLE_NAME(name);
    sqlite3* db;
    sqlite3_stmt* stmt;

    // Open the database file
    if (!open_readonly(&db, path))
        return entries;

    string search_query = sql::search(table, keyword);

    // Prepare row count query
    if(!prepare(db, &stmt, search_query))
        return entries;

    // Execute row count query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        RomEntry entry = {
            .disp = string((char*)sqlite3_column_text(stmt, 1)),
            .path = string((char*)sqlite3_column_text(stmt, 2)),
            .imgpath = string((char*)sqlite3_column_text(stmt, 3)),
            .type = sqlite3_column_int(stmt, 4),
            .ppath = string((char*)sqlite3_column_text(stmt, 5))
        };
        entries.push_back(entry);
    }
    
    if (!isDone(db, rc))
        return entries;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return entries;
}

bool filterEntries(sqlite3* db, string name, string keyword)
{
    if (keyword.length() == 0)
        return true;

    int rc;
    int count = 0;
    sqlite3_stmt* stmt;
    string table = TABLE_NAME(name);

    string search_query = sql::search(table, keyword);
    string filter_query = sql::filter(table, search_query);

    // Prepare filter query
    if(!prepare(db, &stmt, filter_query))
        return false;

    // Execute query
    rc = sqlite3_step(stmt);
    
    if (!isDone(db, rc))
        return false;

    sqlite3_finalize(stmt);
    return true;
}

int countEntries(sqlite3* db, string name)
{
    int rc, count;
    sqlite3_stmt* stmt;
    string table = TABLE_NAME(name);

    // Prepare row count query
    if(!prepare(db, &stmt, sqlite3_mprintf("SELECT count(*) FROM %Q WHERE type=0;", table.c_str())))
        return -1;

    // Execute row count query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    if (!isDone(db, rc))
        return -1;

    sqlite3_finalize(stmt);
    return count;
}
int countEntries(string path, string name)
{
    int count;
    sqlite3* db;

    // Open the database file
    if (!open(&db, path))
        return -1;

    count = countEntries(db, name);

    sqlite3_close(db);
    return count;
}

int filterAndCount(string path, string name, string keyword)
{
    int count;
    sqlite3* db;

    // Open the database file
    if (!open(&db, path))
        return -1;

    if (!filterEntries(db, name, keyword))
        return -1;

    count = countEntries(db, name);

    sqlite3_close(db);
    return count;
}

void clearAll()
{
    subdirForEach(ROMS_PATH, [](string name) {
        string path = CACHE_PATH(name);
        string backup_path = path + ".backup";

        // If cache does not exist, remove backup and exit
        if (!exists(path)) {
            if (exists(backup_path))
                remove(backup_path.c_str());
            return;
        }

        // If no backup is found, do nothing
        if (!exists(backup_path) || countEntries(backup_path, name) <= 0)
            return;
        
        // Restore the cache from backup
        remove(path.c_str());
        rename(backup_path.c_str(), path.c_str());
    });
}

map<string, int> filterAll(string keyword)
{
    clearAll();

    map<string, int> results;

    subdirForEach(ROMS_PATH, [keyword, &results](string name) {
        string path = CACHE_PATH(name);
        string backup_path = path + ".backup";

        // If cache isn't found, we can't filter it
        if (!exists(path))
            return;

        // Create backup of cache (if not empty)
        if (!exists(backup_path) && countEntries(path, name) > 0)
            copyFile(path, backup_path);

        // Filter the cache db and return count
        results[name] = filterAndCount(path, name, keyword);
    });

    return results;
}

map<string, vector<RomEntry>> searchAll(string keyword)
{
    map<string, vector<RomEntry>> results;

    subdirForEach(ROMS_PATH, [keyword, &results](string name) {
        string path = CACHE_PATH(name);

        // If cache isn't found, we can't filter it
        if (!exists(path))
            return;

        // Filter the cache db and return count
        results[name] = searchEntries(name, keyword);
    });

    return results;
}

} // namespace db

#endif // DB_CACHE_HPP__
