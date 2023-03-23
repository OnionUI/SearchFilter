#if !defined(FILTER_HPP__)
#define FILTER_HPP__

#include <algorithm>
#include <regex>
#include <stdio.h>
#include <string>
#include <vector>

using std::regex;
using std::regex_replace;
using std::string;
using std::vector;

#include "../common/ConfigEmu.hpp"
#include "../common/GameJsonEntry.hpp"
#include "../common/db_cache.hpp"
#include "../common/utils.hpp"
#include "../kbinput/keyboard.hpp"

#define PROXY_PATH "../../.tmp_update/proxy.sh"
#define PROXY_RE "\\.\\.\\/\\.\\.\\/.*?\\/proxy\\.sh"
#define ACTIVE_FILTER(emu_path) emu_path + "/active_filter"

#define SCRIPT_FILTER                                                          \
    "#!/bin/sh\ncd /mnt/SDCARD/.tmp_update; ./bin/filter filter \"$2\""
#define SCRIPT_CLEAR_FILTER                                                    \
    "#!/bin/sh\ncd /mnt/SDCARD/.tmp_update; ./bin/filter clear_filter \"$2\""
#define SCRIPT_REFRESH                                                         \
    "#!/bin/sh\ncd /mnt/SDCARD/.tmp_update; ./bin/filter refresh \"$2\""

string commandPath(string path, string cmd)
{
    return path + "/~" + cmd + ".miyoocmd";
}

string cachePath(ConfigEmu config)
{
    string rom_path = fullpath(dirname(config.path), config.rompath);
    return rom_path + "/" + basename(rom_path) + "_cache2.db";
}

void addCommand(sqlite3 *db, string name, string path, string cmd,
                string disp = "")
{
    db::insertRom(db, name,
                  {.disp = disp.length() > 0 ? disp : "~" + cmd,
                   .path = commandPath(path, cmd)});
}

int remove(const string &file_path) { return remove(file_path.c_str()); }

void patchGamelist(string path)
{
    vector<GameJsonEntry> entries = loadGameJsonEntries(path);
    string contents = "";

    for (auto &entry : entries) {
        regex re("^(\\/mnt\\/SDCARD\\/Emu\\/[^\\/]*?\\/)(launch.sh)$");
        entry.launch = regex_replace(entry.launch, re, "$1" PROXY_PATH);
        contents += entry.toJson() + "\n";
    }

    putFile(path, contents);
}

void unpatchGamelist(string path)
{
    vector<GameJsonEntry> entries = loadGameJsonEntries(path);
    string contents = "";

    for (auto &entry : entries) {
        regex re("^(\\/mnt\\/SDCARD\\/Emu\\/[^\\/]*?\\/)(" PROXY_RE ")$");
        entry.launch = regex_replace(entry.launch, re, "$1launch.sh");
        contents += entry.toJson() + "\n";
    }

    putFile(path, contents);
}

void installFilter(void)
{
    vector<ConfigEmu> configs = getEmulatorConfigs();

    for (auto &config : configs) {
        // Only patch configs we can restore, and not already patched
        if (config.launch == "launch.sh") {
            config.launch = PROXY_PATH;

            if (config.extlist != "" &&
                config.extlist.find("miyoocmd") == std::string::npos)
                config.extlist += "|miyoocmd";

            config.save();
        }

        string full_path = dirname(config.path) + "/" + config.rompath;

        if (exists(full_path)) {
            putFile(commandPath(full_path, "Filter"), SCRIPT_FILTER);
            remove(commandPath(full_path, "Clear filter"));
            putFile(commandPath(full_path, "Refresh roms"), SCRIPT_REFRESH);
        }

        sqlite3 *db;
        string name = basename(config.rompath);
        string cache_path = cachePath(config);

        if (!exists(cache_path) || !db::open(&db, cache_path))
            continue;

        db::removeCommands(db, name);
        addCommand(db, name, full_path, "Filter");
        addCommand(db, name, full_path, "Refresh roms");

        sqlite3_close(db);
    }

    // Fix paths of favorites and recentlist
    unpatchGamelist(FAVORITES_PATH);
    unpatchGamelist(RECENTLIST_PATH);
    unpatchGamelist(RECENTLIST_HIDDEN_PATH);

    patchGamelist(FAVORITES_PATH);
    patchGamelist(RECENTLIST_PATH);
    patchGamelist(RECENTLIST_HIDDEN_PATH);
}

void uninstallFilter(void)
{
    vector<ConfigEmu> configs = getEmulatorConfigs();

    for (auto &config : configs) {
        // Skip configs not patched
        if (config.launch == PROXY_PATH)
            config.launch = "launch.sh";

        string full_path = dirname(config.path) + "/" + config.rompath;
        remove(commandPath(full_path, "Filter"));
        remove(commandPath(full_path, "Clear filter"));
        remove(commandPath(full_path, "Refresh roms"));

        sqlite3 *db;
        string name = basename(config.rompath);
        string cache_path = cachePath(config);

        if (!db::open(&db, cache_path))
            continue;

        db::removeCommands(db, name);

        sqlite3_close(db);
    }

    // Fix paths of favorites and recentlist
    unpatchGamelist(FAVORITES_PATH);
    unpatchGamelist(RECENTLIST_PATH);
    unpatchGamelist(RECENTLIST_HIDDEN_PATH);
}

void refreshRoms(string emu_path)
{
    string config_path = emu_path + "/config.json";

    if (!exists(config_path))
        return;

    ConfigEmu config = ConfigEmu::load(config_path);
    string cache_path = cachePath(config);

    remove(cache_path);
}

void clearFilter(string emu_path)
{
    string config_path = emu_path + "/config.json";

    if (!exists(config_path))
        return;

    ConfigEmu config = ConfigEmu::load(config_path);
    string name = basename(config.rompath);
    string cache_path = cachePath(config);

    sqlite3 *db;

    if (!db::open(&db, cache_path)) {
        remove(cache_path);
        return;
    }

    db::clearFilter(db, name);
    remove(ACTIVE_FILTER(emu_path));

    string full_path = dirname(config.path) + "/" + config.rompath;
    db::removeCommands(db, name);
    addCommand(db, name, full_path, "Filter");
    addCommand(db, name, full_path, "Refresh roms");

    remove(commandPath(full_path, "Clear filter"));

    sqlite3_close(db);
}

void applyFilter(Display *display, string emu_path)
{
    string keyword = getFile(ACTIVE_FILTER(emu_path));
    int ec = 0;

    ec = kbinput(display, "Enter keywords", keyword, &keyword);

    if (ec != 0)
        return;

    if (keyword.length() == 0) {
        display->flipText("Removing filter...");
        clearFilter(emu_path);
        return;
    }

    SDL_Surface *filter_icon = IMG_Load("res/icon_filter.png");
    display->clear();
    display->blit(filter_icon,
                  {320 - filter_icon->w / 2, 150 - filter_icon->h / 2,
                   filter_icon->w, filter_icon->h});
    display->centerText("Applying filter...", {320, 240},
                        display->fonts.display);
    display->flip();
    SDL_FreeSurface(filter_icon);
    filter_icon = NULL;

    string config_path = emu_path + "/config.json";

    if (!exists(config_path))
        return;

    ConfigEmu config = ConfigEmu::load(config_path);
    string name = basename(config.rompath);
    string cache_path = cachePath(config);

    sqlite3 *db;

    if (!db::open(&db, cache_path)) {
        remove(cache_path);
        return;
    }

    db::filterEntries(db, name, keyword);
    putFile(ACTIVE_FILTER(emu_path), keyword);

    string full_path = dirname(config.path) + "/" + config.rompath;
    db::removeCommands(db, name);
    addCommand(db, name, full_path, "Filter", "~Filter: " + keyword);
    addCommand(db, name, full_path, "Clear filter");
    addCommand(db, name, full_path, "Refresh roms");

    putFile(commandPath(full_path, "Clear filter"), SCRIPT_CLEAR_FILTER);

    int total_lines = db::countRootEntries(db, name);
    db::addEmptyLines(db, name, total_lines);

    sqlite3_close(db);
}

#endif // FILTER_HPP__
