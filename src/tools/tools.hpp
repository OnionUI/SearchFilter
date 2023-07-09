#ifndef TOOLS_HPP__
#define TOOLS_HPP__

#include <algorithm>
#include <dirent.h>
#include <json/json.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

using std::any_of;
using std::count;
using std::sort;
using std::string;
using std::unordered_map;
using std::vector;

#include "../common/ConfigEmu.hpp"
#include "../common/GameJsonEntry.hpp"
#include "../common/utils.hpp"

#define APP_ROOT "/mnt/SDCARD/Emu/SEARCH/../../App/Search"
#define LAUNCH_PATH APP_ROOT "/launch.sh"
#define TEMP_FAV_FILE "/tmp/__favfix_favourite.json"

namespace tools {

void fixFavorites(void)
{
    vector<GameJsonEntry> favorites = loadGameJsonEntries(FAVORITES_PATH);
    vector<string> added_paths;
    vector<string> added_labels;
    string contents = "";

    vector<string> emu_labels;
    unordered_map<string, ConfigEmu> emu_configs_lookup;

    for (auto &config : getEmulatorConfigs()) {
        emu_labels.push_back(config.label);
        emu_configs_lookup[dirname(config.path)] = config;
    }

    for (auto &entry : favorites) {
        string emuname = basename(entry.emupath);

        if (emu_configs_lookup.find(entry.emupath) == emu_configs_lookup.end())
            continue;

        char actual_path[PATH_MAX + 1];
        realpath(entry.rompath.c_str(), actual_path);
        string abs_path(actual_path);

        if (std::count(added_paths.begin(), added_paths.end(), abs_path) == 0) {
            string romname = removeExtension(basename(entry.rompath));

            if (std::count(emu_labels.begin(), emu_labels.end(), entry.label) != 0)
                entry.label = romname;

            if (emu_configs_lookup[entry.emupath].imgpath.length() > 0) {
                entry.imgpath = entry.emupath + "/" + emu_configs_lookup[entry.emupath].imgpath + "/" + romname + ".png";
            }

            added_paths.push_back(abs_path);
            contents += entry.toJson() + "\n";
        }
        else {
            std::cout << "Removed duplicate: " << abs_path << std::endl;
        }
    }

    string prev_contents = getFile(FAVORITES_PATH) + "\n";

    if (contents.compare(prev_contents) != 0) {
        putFile(FAVORITES_PATH, contents);
        std::cout << "Favorites file was modified" << std::endl;
    }
    else {
        std::cout << "Favorites file was not modified" << std::endl;
    }
}

void sortFavorites(bool (*sort_function)(GameJsonEntry, GameJsonEntry) = NULL)
{
    vector<GameJsonEntry> favorites = loadGameJsonEntries(FAVORITES_PATH);
    string contents = "";

    if (sort_function == NULL)
        sort_function = [](GameJsonEntry a, GameJsonEntry b) {
            return tolower(a.label) < tolower(b.label);
        };

    sort(favorites.begin(), favorites.end(), sort_function);

    for (auto &entry : favorites)
        contents += entry.toJson() + "\n";

    putFile(FAVORITES_PATH, contents);
}

void sortFavoritesBySystem(void)
{
    sortFavorites([](GameJsonEntry a, GameJsonEntry b) {
        if (a.label[0] == '~') {
            if (b.label[0] == '~')
                return tolower(a.label) < tolower(b.label);
            return false;
        }
        else if (b.label[0] == '~')
            return true;
        return tolower(basename(a.emupath)) < tolower(basename(b.emupath));
    });
}

void addShortcut(vector<GameJsonEntry> *favorites, string label, string cmd)
{
    auto hasShortcut = [cmd](GameJsonEntry entry) {
        return entry.rompath == cmd;
    };
    if (any_of(favorites->begin(), favorites->end(), hasShortcut))
        return;
    favorites->push_back(
        {label, LAUNCH_PATH, 5, APP_ROOT "/data/~Tools/" + cmd + ".miyoocmd"});
}

void addFavoritesTools(void)
{
    vector<GameJsonEntry> favorites = loadGameJsonEntries(FAVORITES_PATH);
    string contents = "";

    addShortcut(&favorites, "~Fix boxart", "1. Fix favorites boxart");
    addShortcut(&favorites, "~Sort alphabetically", "2. Sort favorites (A-Z)");
    addShortcut(&favorites, "~Sort by system", "3. Sort favorites (by system)");

    for (auto &entry : favorites)
        contents += entry.toJson() + "\n";

    putFile(FAVORITES_PATH, contents);
}

void cleanRecentList(string recentlist_path, bool clean_all)
{
    if (!exists(recentlist_path))
        return;

    vector<GameJsonEntry> recents = loadGameJsonEntries(recentlist_path);
    string contents = "";

    for (auto &entry : recents) {
        if (entry.rompath.find(":") != string::npos) {
            vector<string> tokens = split(entry.rompath, ":");
            entry.launch = tokens[0];
            entry.rompath = tokens[1];
        }
        else if (entry.launch == LAUNCH_PATH ||
                 getExtension(entry.rompath) == "miyoocmd" ||
                 (clean_all && entry.type == 3)) {
            continue;
        }
        contents += entry.toJson() + "\n";
    }

    putFile(recentlist_path, contents);
}

} // namespace tools

#endif // TOOLS_HPP__
