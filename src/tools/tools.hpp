#if !defined(TOOLS_HPP__)
#define TOOLS_HPP__

#include <string>
#include <vector>
#include <algorithm>
#include <json/json.h>

using std::sort;
using std::any_of;
using std::string;
using std::vector;

#include "../common/utils.hpp"
#include "../common/GameJsonEntry.hpp"

#define APP_ROOT "/mnt/SDCARD/Emu/SEARCH/../../App/SearchFilter"
#define LAUNCH_PATH APP_ROOT "/launch.sh"

namespace tools {

void fixFavoritesBoxart(void)
{
    vector<GameJsonEntry> favorites = loadGameJsonEntries(FAVORITES_PATH);
    string contents = "";

    for (auto &entry : favorites) {
        // Entries are automatically fixed when loaded
        contents += entry.toJson() + "\n";
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

void addShourtcut(vector<GameJsonEntry>* favorites, string label, string cmd)
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

    addShourtcut(&favorites, "~Fix boxart", "1. Fix favorites boxart");
    addShourtcut(&favorites, "~Sort alphabetically", "2. Sort favorites (A-Z)");
    addShourtcut(&favorites, "~Sort by system", "3. Sort favorites (by system)");

    for (auto &entry : favorites)
        contents += entry.toJson() + "\n";

    putFile(FAVORITES_PATH, contents);
}

void cleanRecentList(bool only_garbage)
{
    vector<GameJsonEntry> recents = loadGameJsonEntries(RECENTLIST_PATH);
    string contents = "";

    for (auto &entry : recents) {
        if (entry.rompath.find(":") != string::npos) {
            vector<string> tokens = split(entry.rompath, ":");
            entry.launch = tokens[0];
            entry.rompath = tokens[1];
        }
        else if (entry.launch == LAUNCH_PATH || getExtension(entry.rompath) == "miyoocmd" || (!only_garbage && entry.type == 3)) {
            continue;
        }
        contents += entry.toJson() + "\n";
    }

    putFile(RECENTLIST_PATH, contents);
}

} // namespace tools

#endif // TOOLS_HPP__
