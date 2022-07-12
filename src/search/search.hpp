#if !defined(SEARCH_HPP__)
#define SEARCH_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
using std::string;
using std::to_string;
using std::vector;

#include "../common/display.hpp"
#include "../common/utils.hpp"
#include "../common/db_cache.hpp"
#include "../common/config.hpp"

#define ACTIVE_SEARCH "active_search"

const string DB_NAME = "data";
const string DB_DIR = fullpath(DB_NAME);
const string DB_PATH = DB_NAME + "/" + CACHE_NAME(DB_NAME);

static SDL_Surface* search_icon = NULL;

void updateDisplay(Display *display, string message)
{
    display->clear();

    if (search_icon)
        display->blit(search_icon, {
            320 - search_icon->w / 2,
            180 - search_icon->h / 2,
            search_icon->w,
            search_icon->h
        });

    display->centerText(message, {320, 300}, display->fonts.display);
    display->flip();
}

void addTools(sqlite3* db)
{
    db::insertRom(db, DB_NAME, {
        .disp = "~Tools",
        .path = "",
        .imgpath = "",
        .type = 1
    });

    auto addTool = [db](string disp, string cmd) {
        db::insertRom(db, DB_NAME, {
            .disp = disp,
            .path = cmd,
            .imgpath = DB_DIR + "/" + disp + ".png",
            .type = 0,
            .ppath = "~Tools"
        });
    };

    addTool("Add tools to favorites", "favtools");
    addTool("Fix favorites boxart", "boxart");
    addTool("Sort favorites", "favsort");
}

void addEmptyLines(sqlite3* db, int &total_lines)
{
    for (; total_lines < 6; total_lines++)
        db::insertRom(db, DB_NAME, {
            .disp = "~",
            .path = "noop"
        });
}

void performSearch(Display* display, string keyword)
{
    if (exists(DB_PATH))
        remove(DB_PATH.c_str());

    if (!db::create(DB_PATH, DB_NAME)) {
        std::cerr << "Couldn't create database" << std::endl;
        return;
    }

    sqlite3* db;
    int total_lines = 0;

    // Open the database file
    if (!db::open(&db, DB_PATH))
        return;

    keyword = trim(keyword);

    if (keyword.length() == 0) {
        db::insertRom(db, DB_NAME, {
            .disp = "Enter search term...",
            .path = "search",
            .imgpath = DB_DIR + "/Imgs/Enter search term....png"
        });
        addTools(db);
        total_lines += 2;
        addEmptyLines(db, total_lines);
        sqlite3_close(db);
        return;
    }

    vector<ConfigEmu> configs = getEmulatorConfigs();
    int total = 0;

    int current_emu = 0;
    int total_emu = configs.size();

    vector<string> missing_caches;

    for (auto &config : configs) {
        current_emu++;
        updateDisplay(display, "Searching: " + to_string(current_emu) + "/" + to_string(total_emu));

        string rom_path = fullpath(config.path, config.rompath);
        string launch_cmd = fullpath(config.path) + "/" + config.launch;
        string name = basename(rom_path);
        string cache_path = rom_path + "/" + CACHE_NAME(name);

        if (name.length() == 0)
            continue;

        if (!exists(cache_path)) {
            missing_caches.push_back(trim(config.label));
            continue;
        }

        vector<RomEntry> result = db::searchEntries(name, keyword);
        int subtotal = result.size();
        string label = trim(config.label) + " (" + to_string(subtotal) + ")";

        if (subtotal <= 0)
            continue;

        total += subtotal;

        db::insertRom(db, DB_NAME, {
            .disp = label,
            .path = rom_path,
            .imgpath = rom_path,
            .type = 1
        });
        total_lines++;

        for (auto &entry : result) {
            string path = launch_cmd + ":" + entry.path;
            db::insertRom(db, DB_NAME, {
                .disp = entry.disp,
                .path = path,
                .imgpath = entry.imgpath,
                .type = 0,
                .ppath = label
            });
        }
    }

    updateDisplay(display, "Done");

    string all_label = "All consoles (" + to_string(total) + ")";

    db::insertRom(db, DB_NAME, {
        .disp = all_label,
        .path = "",
        .imgpath = "",
        .type = 1
    });
    total_lines++;

    if (total == 0) {
        db::insertRom(db, DB_NAME, {
            .disp = "No results",
            .path = "clear",
            .imgpath = DB_DIR + "/../res/icon_unavailable.png",
            .type = 0,
            .ppath = all_label
        });
    }
    else {
        // Copy all rows of type=0 and change ppath -> all_label
        db::duplicateResults(db, DB_NAME, all_label);
    }

    if (missing_caches.size() > 0) {
        string cache_missing_label = "~Missing consoles (" + to_string(missing_caches.size()) + ")";

        db::insertRom(db, DB_NAME, {
            .disp = cache_missing_label,
            .path = "",
            .imgpath = "",
            .type = 1
        });
        total_lines++;

        for (auto &label : missing_caches) {
            db::insertRom(db, DB_NAME, {
                .disp = label,
                .path = "noop",
                .imgpath = DB_DIR + "/../res/help_unavailable.png",
                .type = 0,
                .ppath = cache_missing_label
            });
        }
    }

    addTools(db);
    total_lines++;

    db::insertRom(db, DB_NAME, {
        .disp = "Clear search",
        .path = "clear",
        .imgpath = DB_DIR + "/../res/help_clear_search.png"
    });
    total_lines++;

    db::insertRom(db, DB_NAME, {
        .disp = "Search: " + keyword,
        .path = "search",
        .imgpath = DB_DIR + "/Imgs/Enter search term....png"
    });
    total_lines++;

    addEmptyLines(db, total_lines);

    sqlite3_close(db);
}

#endif // SEARCH_HPP__
