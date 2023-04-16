#if !defined(DB_CACHE_HPP__)
#define DB_CACHE_HPP__

#include <algorithm>
#include <iostream>
#include <map>
#include <sqlite3/sqlite3.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

using std::map;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

#include "utils.hpp"

#define CACHE_NAME(name) name + "_cache6.db"
#define TABLE_NAME(name) name + "_roms"

struct RomEntry {
    string label = "";
    string path = "";
    string imgpath = "";
    int type = 0;
    string ppath = ".";

    RomEntry changePpath(string _ppath)
    {
        return {.label = label,
                .path = path,
                .imgpath = imgpath,
                .type = type,
                .ppath = _ppath};
    }
};

namespace db::sql {

string search(string table, const string &keyword_str)
{
    string sql = sqlite3_mprintf(
        "SELECT * FROM %Q WHERE path NOT LIKE '%%.miyoocmd'", table.c_str());
    vector<string> keywords = split(keyword_str, " ");

    bool first = true;
    int count = keywords.size();

    for (int i = 0; i < count; i++) {
        string keyword = trim(keywords[i]);
        if (keyword.length() == 0)
            continue;
        sql += sqlite3_mprintf(" AND pinyin LIKE '%%%q%%'", keyword.c_str());
        first = false;
    }

    return sql;
}

string subdirs(string table, string search_query)
{
    string sql = "SELECT id, path AS _path FROM %Q WHERE type=1 AND EXISTS ("
                 "SELECT id FROM %Q WHERE path LIKE _path || '%%' AND id IN ("
                 "SELECT id FROM (%s)))";
    return string(sqlite3_mprintf(sql.c_str(), table.c_str(), table.c_str(),
                                  search_query.c_str()));
}

string filter(string table, string search_query)
{
    string sql = "UPDATE %Q SET ppath = ppath || '<filtered>' WHERE "
                 "path NOT LIKE '%%.miyoocmd' AND "
                 "ppath NOT LIKE '%%<filtered>' AND "
                 "(type=0 AND id NOT IN (SELECT id FROM (%s)) OR "
                 "type=1 AND id NOT IN (SELECT id FROM (%s)));";
    string subdir_query = subdirs(table, search_query);
    return string(sqlite3_mprintf(sql.c_str(), table.c_str(),
                                  search_query.c_str(), subdir_query.c_str()));
}

string clear_filter(string name)
{
    string sql = "UPDATE %Q SET ppath = SUBSTRING(ppath,1,length(ppath)-10) "
                 "WHERE ppath LIKE '%%<filtered>';";
    string table = TABLE_NAME(name);
    return string(sqlite3_mprintf(sql.c_str(), table.c_str()));
}

string delete_commands(string name)
{
    string sql = "DELETE FROM %Q WHERE path LIKE '%%.miyoocmd' AND "
                 "(disp LIKE '~Filter%%' OR disp='~Refresh roms' OR "
                 "disp='~Clear filter')";
    string table = TABLE_NAME(name);
    return string(sqlite3_mprintf(sql.c_str(), table.c_str()));
}

string create_roms_table(string name)
{
    string sql = "CREATE TABLE %Q ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "disp TEXT NOT NULL,"
                 "path TEXT NOT NULL,"
                 "imgpath TEXT NOT NULL,"
                 "type INTEGER DEFAULT 0,"
                 "ppath TEXT NOT NULL,"
                 "pinyin TEXT NOT NULL,"
                 "cpinyin TEXT NOT NULL)";
    string table = TABLE_NAME(name);
    return string(sqlite3_mprintf(sql.c_str(), table.c_str()));
}

string insert(string name, RomEntry entry)
{
    string sql = "INSERT INTO %Q (disp, path, imgpath, type, ppath, pinyin, cpinyin) "
                 "VALUES (%Q, %Q, %Q, %q, %Q, %Q, '');";
    string table = TABLE_NAME(name);
    return string(
        sqlite3_mprintf(sql.c_str(), table.c_str(), entry.label.c_str(),
                        entry.path.c_str(), entry.imgpath.c_str(),
                        to_string(entry.type).c_str(), entry.ppath.c_str(), entry.label.c_str()));
}

string dupChangePpath(string name, string ppath)
{
    string sql = "INSERT INTO %Q (disp, path, imgpath, type, ppath, pinyin, cpinyin) "
                 "SELECT disp, path, imgpath, type, %Q, pinyin, cpinyin FROM %Q WHERE type=0 "
                 "AND path!='nocache';";
    string table = TABLE_NAME(name);
    return string(sqlite3_mprintf(sql.c_str(), table.c_str(), ppath.c_str(),
                                  table.c_str()));
}

} // namespace db::sql

namespace db {

bool open(sqlite3 **db, string path)
{
    if (sqlite3_open(path.c_str(), db) != SQLITE_OK) {
        std::cerr << "ERROR: can't open database: " << sqlite3_errmsg(*db)
                  << " [" << path << "]" << std::endl;
        sqlite3_close(*db);
        return false;
    }
    return true;
}

bool open_readonly(sqlite3 **db, string path)
{
    if (sqlite3_open_v2(path.c_str(), db, SQLITE_OPEN_READONLY, NULL) !=
        SQLITE_OK) {
        std::cerr << "ERROR: can't open database: " << sqlite3_errmsg(*db)
                  << " [" << path << "]" << std::endl;
        sqlite3_close(*db);
        return false;
    }
    return true;
}

bool prepare(sqlite3 *db, sqlite3_stmt **stmt, const char *query)
{
    if (sqlite3_prepare_v2(db, query, -1, stmt, NULL) != SQLITE_OK) {
        std::cerr << "ERROR: while compiling sql: " << sqlite3_errmsg(db)
                  << std::endl;
        std::cerr << "sql: " << query << std::endl;
        sqlite3_finalize(*stmt);
        sqlite3_close(db);
        return false;
    }
    return true;
}
bool prepare(sqlite3 *db, sqlite3_stmt **stmt, string query)
{
    return prepare(db, stmt, query.c_str());
}

bool isDone(sqlite3 *db, int rc)
{
    if (rc != SQLITE_DONE) {
        std::cerr << "ERROR: while performing sql: " << sqlite3_errmsg(db)
                  << std::endl;
        std::cerr << "Return code: " << to_string(rc) << std::endl;
        sqlite3_close(db);
        return false;
    }
    return true;
}

bool execSql(sqlite3 *db, string sql)
{
    int rc;
    sqlite3_stmt *stmt;

    // Prepare SQL statement
    if (!prepare(db, &stmt, sql))
        return false;

    // Execute statement
    rc = sqlite3_step(stmt);

    if (!isDone(db, rc))
        return false;

    sqlite3_finalize(stmt);
    return true;
}

bool create(string path, string name)
{
    int rc;
    sqlite3 *db;

    // Open the database file
    if (!open(&db, path))
        return false;

    // Create roms table
    if (!execSql(db, sql::create_roms_table(name)))
        return false;

    sqlite3_close(db);
    return true;
}

bool insertRom(sqlite3 *db, string name, RomEntry entry)
{
    return execSql(db, sql::insert(name, entry));
}

bool duplicateResults(sqlite3 *db, string name, string ppath)
{
    return execSql(db, sql::dupChangePpath(name, ppath));
}

bool removeCommands(sqlite3 *db, string name)
{
    return execSql(db, sql::delete_commands(name));
}

vector<RomEntry> searchEntries(string path, string keyword)
{
    int rc;
    vector<RomEntry> entries;

    if (keyword.length() == 0)
        return entries;

    string name = basename(dirname(path));
    string table = TABLE_NAME(name);
    sqlite3 *db;
    sqlite3_stmt *stmt;

    // Open the database file
    if (!open_readonly(&db, path))
        return entries;

    string search_query = sql::search(table, keyword);

    // Prepare row count query
    if (!prepare(db, &stmt, search_query))
        return entries;

    // Execute row count query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        RomEntry entry = {
            .label = string((char *)sqlite3_column_text(stmt, 1)),
            .path = string((char *)sqlite3_column_text(stmt, 2)),
            .imgpath = string((char *)sqlite3_column_text(stmt, 3)),
            .type = sqlite3_column_int(stmt, 4),
            .ppath = string((char *)sqlite3_column_text(stmt, 5))};
        entries.push_back(entry);
    }

    if (!isDone(db, rc))
        return entries;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return entries;
}

bool filterEntries(sqlite3 *db, string name, string keyword)
{
    if (keyword.length() == 0)
        return true;
    string table = TABLE_NAME(name);
    string search_query = sql::search(table, keyword);
    string filter_query = sql::filter(table, search_query);
    return execSql(db, filter_query);
}

bool clearFilter(sqlite3 *db, string name)
{
    return execSql(db, sql::clear_filter(name));
}

int countRootEntries(sqlite3 *db, string name)
{
    int rc, count;
    sqlite3_stmt *stmt;
    string table = TABLE_NAME(name);
    string sql = "SELECT count(*) FROM %Q WHERE ppath='.';";

    // Prepare row count query
    if (!prepare(db, &stmt, sqlite3_mprintf(sql.c_str(), table.c_str())))
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

void addEmptyLines(sqlite3 *db, string name, int &total_lines)
{
    for (; total_lines < 6; total_lines++)
        insertRom(db, name, {.label = "~", .path = "noop.miyoocmd"});
}

} // namespace db

#endif // DB_CACHE_HPP__
