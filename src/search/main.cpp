#if !defined(VERSION)
#define VERSION ""
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
using std::string;
using std::vector;
using std::map;

#include "../common/display.hpp"
#include "../common/sysutils.hpp"
#include "../common/db_cache.hpp"
#include "../common/config.hpp"
#include "../kbinput/keyboard.hpp"

#define ACTIVE_SEARCH "active_search"

void flipText(Display* display, string text)
{
    display->clear();
    display->centerText(text, {320, 240}, display->fonts.display);
    display->flip();
}

void performSearch(Display* display, string keyword)
{
    string db_name = "data";
    string db_dir = fullpath(db_name);
    string db_path = db_name + "/" + CACHE_NAME(db_name);

    if (exists(db_path))
        remove(db_path.c_str());

    if (!db::create(db_path, db_name)) {
        std::cerr << "Could't create database" << std::endl;
        return;
    }

    sqlite3* db;

    // Open the database file
    if (!db::open(&db, db_path))
        return;

    if (keyword.length() == 0) {
        db::insertRom(db, db_name, {
            .disp = "[Enter search term...]",
            .path = "search",
            .imgpath = db_dir + "/../res/icon_search_field.png"
        });
        return;
    }

    vector<ConfigEmu> configs = getEmulatorConfigs();
    vector<RomEntry> all_results;
    int total = 0;

    int current_emu = 0;
    int total_emu = configs.size();

    for (auto &config : configs) {
        current_emu++;
        string rom_path = fullpath(config.path, config.rompath);
        string launch_cmd = "'" + fullpath(config.path) + "/" + config.launch + "'";
        string name = basename(rom_path);
        string cache_path = rom_path + "/" + CACHE_NAME(name);

        if (name.length() == 0)
            continue;

        if (!exists(cache_path)) {
            db::insertRom(db, db_name, {
                .disp = config.label + " (Not cached)",
                .path = rom_path,
                .imgpath = db_dir + "/../res/icon_unavailable.png",
                .type = 0
            });
            continue;
        }

        vector<RomEntry> result = db::searchEntries(name, keyword);
        int subtotal = result.size();
        string label = config.label + " (" + std::to_string(subtotal) + ")";

        if (subtotal <= 0)
            continue;

        total += subtotal;

        db::insertRom(db, db_name, {
            .disp = label,
            .path = rom_path,
            .imgpath = rom_path,
            .type = 1
        });

        for (auto &entry : result) {
            string path = launch_cmd + " '" + entry.path + "'";
            RomEntry _entry = {
                .disp = entry.disp,
                .path = path,
                .imgpath = entry.imgpath,
                .ppath = label
            };
            db::insertRom(db, db_name, _entry);
            all_results.push_back(_entry);
        }

        flipText(display, "Searching... " + std::to_string(current_emu) + "/" + std::to_string(total_emu));
    }

    flipText(display, "Searching... " + std::to_string(total_emu) + "/" + std::to_string(total_emu));

    db::insertRom(db, db_name, {
        .disp = "[Clear search]",
        .path = "clear",
        .imgpath = db_dir + "/../res/icon_clear_search.png"
    });

    db::insertRom(db, db_name, {
        .disp = "[Search: '" + keyword + "']",
        .path = "search",
        .imgpath = db_dir + "/../res/icon_search_field.png"
    });

    string all_label = "All results (" + std::to_string(total) + ")";
    db::insertRom(db, db_name, {
        .disp = all_label,
        .path = "",
        .imgpath = "",
        .type = 1
    });

    for (auto &entry : all_results) {
        db::insertRom(db, db_name, entry.changePpath(all_label));
    }

    sqlite3_close(db);
}

int main(int argc, char** argv)
{
    if (argc >= 2 && std::string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    int ec = 0;
    Display* display = new Display();
    string keyword = getFile(ACTIVE_SEARCH);

    if (argc >= 2 && std::string(argv[1]) == "clear")
        keyword = "";
    else
        ec = kbinput(display, "Search:", keyword, &keyword);

    if (ec == 0) {
        if (keyword.length() > 0) {
            putFile(ACTIVE_SEARCH, keyword);
            flipText(display, "Searching...");
        }
        else {
            remove(ACTIVE_SEARCH);
        }
        performSearch(display, keyword);
    }

    delete display;
    return 0;
}
