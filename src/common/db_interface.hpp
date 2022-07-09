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
using std::stringstream;
using std::map;
using std::vector;

#define ROMS_PATH "/mnt/SDCARD/Roms"
#define ROM_PATH(name) ROMS_PATH "/" + name
#define CACHE_PATH(name) ROM_PATH(name) + "/" + name + "_cache2.db"
#define TABLE_NAME(name) name + "_roms"

struct RomEntry
{
    string disp = "";
    string path = "";
    string imgpath = "";
    int type = 0;
    string ppath = ".";
};

bool db_open(sqlite3** db, string db_path)
{
    if (sqlite3_open(db_path.c_str(), db) != SQLITE_OK) {
        std::cerr << "ERROR: can't open database: " << sqlite3_errmsg(*db) << " [" << db_path << "]" << std::endl;
        sqlite3_close(*db);
        return false;
    }

    return true;
}

bool db_open_readonly(sqlite3** db, string db_path)
{
    if (sqlite3_open_v2(db_path.c_str(), db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        std::cerr << "ERROR: can't open database: " << sqlite3_errmsg(*db) << " [" << db_path << "]" << std::endl;
        sqlite3_close(*db);
        return false;
    }

    return true;
}

bool db_prepare(sqlite3* db, sqlite3_stmt** stmt, const char* query)
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
bool db_prepare(sqlite3* db, sqlite3_stmt** stmt, string query)
{
    return db_prepare(db, stmt, query.c_str());
}

bool db_isDone(sqlite3* db, int rc)
{
    if (rc != SQLITE_DONE) {
        std::cerr << "ERROR: while performing sql: " << sqlite3_errmsg(db) << std::endl;
        std::cerr << "Return code: " << std::to_string(rc) << std::endl;
        return false;
    }

    return true;
}

string db_sql_search(string table, string keyword)
{
    string sql;
    stringstream query;
    bool first = true;
    char *ptr;
    char* str = (char*)keyword.c_str();

    query << sqlite3_mprintf("SELECT * FROM %Q WHERE", table.c_str());

    // Split the keywords at every ' ' (space)
    ptr = strtok(str, " ");
    while (ptr) {
        if (!first)
            query << " AND";
        query << sqlite3_mprintf(" disp LIKE '%%%q%%'", ptr);
        ptr = strtok(NULL, " ");
        first = false;
    }

    sql = query.str();

    return sql.c_str();
}

string db_sql_subdirs(string table, string search_query)
{
    string sql = 
        "SELECT id, path AS _path FROM %Q WHERE type=1 AND EXISTS ("
            "SELECT id FROM %Q WHERE path LIKE _path || '%%' AND id IN ("
                "SELECT id FROM (%s)))";
    return string(sqlite3_mprintf(sql.c_str(), table.c_str(), table.c_str(), search_query.c_str()));
}

string db_sql_filter(string table, string search_query)
{
    string sql =
        "DELETE FROM %Q WHERE "
            "type=0 AND id NOT IN (SELECT id FROM (%s)) OR "
            "type=1 AND id NOT IN (SELECT id FROM (%s));";
    string subdir_query = db_sql_subdirs(table, search_query);
    return string(sqlite3_mprintf(sql.c_str(), table.c_str(), search_query.c_str(), subdir_query.c_str()));
}

string db_sql_create_roms_table(string name)
{
    string sql =
        "CREATE TABLE %Q ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "disp TEXT NOT NULL,"
            "path TEXT NOT NULL,"
            "imgpath TEXT NOT NULL,"
            "type INTEGER DEFAULT 0,"
            "ppath TEXT NOT NULL)";
    return string(sqlite3_mprintf(sql.c_str(), TABLE_NAME(name)));
}

bool db_create(string db_path, string name)
{
    int rc;
    sqlite3* db;
    sqlite3_stmt* stmt;

    // Open the database file
    if (!db_open(&db, db_path))
        return false;

    // Create roms table
    if(!db_prepare(db, &stmt, db_sql_create_roms_table(name)))
        return false;
    rc = sqlite3_step(stmt);
    
    if (!db_isDone(db, rc))
        return false;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

vector<RomEntry> db_searchEntries(string name, string keyword)
{
    int rc;
    vector<RomEntry> entries;

    if (keyword.length() == 0)
        return entries;

    string db_path = CACHE_PATH(name);
    string table = TABLE_NAME(name);
    sqlite3* db;
    sqlite3_stmt* stmt;

    // Open the database file
    if (!db_open_readonly(&db, db_path))
        return entries;

    string search_query = db_sql_search(table, keyword);

    // Prepare row count query
    if(!db_prepare(db, &stmt, search_query))
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
    
    if (!db_isDone(db, rc))
        return entries;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return entries;
}

bool db_filterEntries(sqlite3* db, string name, string keyword)
{
    if (keyword.length() == 0)
        return true;

    int rc;
    int count = 0;
    sqlite3_stmt* stmt;
    string table = TABLE_NAME(name);

    string search_query = db_sql_search(table, keyword);
    string filter_query = db_sql_filter(table, search_query);

    // Prepare filter query
    if(!db_prepare(db, &stmt, filter_query))
        return false;

    // Execute query
    rc = sqlite3_step(stmt);
    
    if (!db_isDone(db, rc))
        return false;

    sqlite3_finalize(stmt);
    return true;
}

int db_countEntries(sqlite3* db, string name)
{
    int rc, count;
    sqlite3_stmt* stmt;
    string table = TABLE_NAME(name);

    // Prepare row count query
    if(!db_prepare(db, &stmt, sqlite3_mprintf("SELECT count(*) FROM %Q WHERE type=0;", table.c_str())))
        return -1;

    // Execute row count query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    if (!db_isDone(db, rc))
        return -1;

    sqlite3_finalize(stmt);
    return count;
}
int db_countEntries(string db_path, string name)
{
    int count;
    sqlite3* db;

    // Open the database file
    if (!db_open(&db, db_path))
        return -1;

    count = db_countEntries(db, name);

    sqlite3_close(db);
    return count;
}

int db_filterAndCount(string db_path, string name, string keyword)
{
    int count;
    sqlite3* db;

    // Open the database file
    if (!db_open(&db, db_path))
        return -1;

    if (!db_filterEntries(db, name, keyword))
        return -1;

    count = db_countEntries(db, name);

    sqlite3_close(db);
    return count;
}

void db_clearAll()
{
    subdirForEach<int>(ROMS_PATH, [](string name) {
        string db_path = CACHE_PATH(name);
        string backup_path = db_path + ".backup";

        // If cache does not exist, remove backup and exit
        if (!exists(db_path)) {
            if (exists(backup_path))
                remove(backup_path.c_str());
            return -1;
        }

        // If no backup is found, do nothing
        if (!exists(backup_path) || db_countEntries(backup_path, name) <= 0)
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

    return subdirForEach<int>(ROMS_PATH, [keyword](string name) {
        string db_path = CACHE_PATH(name);
        string backup_path = db_path + ".backup";

        // If cache isn't found, we can't filter it
        if (!exists(db_path))
            return -1;

        // Create backup of cache (if not empty)
        if (!exists(backup_path) && db_countEntries(db_path, name) > 0)
            copyFile(db_path, backup_path);

        // Filter the cache db and return count
        return db_filterAndCount(db_path, name, keyword);
    });
}

map<string, vector<RomEntry>> db_searchAll(string keyword)
{
    map<string, vector<RomEntry>> results = subdirForEach<vector<RomEntry>>(ROMS_PATH, [keyword](string name) {
        string db_path = CACHE_PATH(name);

        // If cache isn't found, we can't filter it
        if (!exists(db_path))
            return vector<RomEntry>();

        // Filter the cache db and return count
        return db_searchEntries(name, keyword);
    });

    return results;
}
