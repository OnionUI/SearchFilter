#ifndef TOOLS_HPP__
#define TOOLS_HPP__

#include <string>
#include <vector>
#include <algorithm>
#include <json/json.h>
#include <unordered_map>

using std::sort;
using std::any_of;
using std::string;
using std::vector;
using std::unordered_map;
using std::count;

#include "../common/utils.hpp"
#include "../common/GameJsonEntry.hpp"
#include "../common/ConfigEmu.hpp"

#define APP_ROOT "/mnt/SDCARD/Emu/SEARCH/../../App/SearchFilter"
#define LAUNCH_PATH APP_ROOT "/launch.sh"

namespace tools {

void fixFavorites(void)
{
    vector<GameJsonEntry> favorites = loadGameJsonEntries(FAVORITES_PATH);
    unordered_map<string, ConfigEmu> emu_configs;
    vector<string> added_rompaths;
    vector<string> added_labels;
    string contents = "";

    for (auto &entry : favorites) {
        if (emu_configs.find(entry.emupath) == emu_configs.end())
            emu_configs[entry.emupath] = getEmulatorConfig(entry.emupath);

        string romname = removeExtension(basename(entry.rompath));

        entry.label = romname;

        string imgpath = entry.emupath + "/";
        if (emu_configs[entry.emupath].imgpath.length() > 0)
            imgpath += emu_configs[entry.emupath].imgpath + "/";
        imgpath += romname + ".png";
        entry.imgpath = imgpath;


        if (std::count(added_rompaths.begin(), added_rompaths.end(), entry.rompath) == 0) {
            int num = 1;
            string label = entry.label;
            while (std::count(added_labels.begin(), added_labels.end(), label) != 0)
                label = entry.label + " (" + to_string(++num) + ")";
            if (num > 1)
                entry.label = label;
            added_labels.push_back(entry.label);
            added_rompaths.push_back(entry.rompath);
            std::cout << entry.label << ": " << imgpath << std::endl;
            contents += entry.toJson() + "\n";
        }
    }

    putFile(FAVORITES_PATH, contents);
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

void addShortcut(vector<GameJsonEntry>* favorites, string label, string cmd)
{
    auto hasShortcut = [cmd](GameJsonEntry entry){
        return entry.rompath == cmd;
    };
    if (any_of(favorites->begin(), favorites->end(), hasShortcut))
        return;
    favorites->push_back({label, LAUNCH_PATH, 5, APP_ROOT "/data/~Tools/" + cmd + ".miyoocmd"});
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
        else if (entry.launch == LAUNCH_PATH || getExtension(entry.rompath) == "miyoocmd" || (clean_all && entry.type == 3)) {
            continue;
        }
        contents += entry.toJson() + "\n";
    }

    putFile(recentlist_path, contents);
}

} // namespace tools

#endif // TOOLS_HPP__
