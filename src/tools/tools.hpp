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
#include "../common/game_entry.hpp"

#define APP_ROOT "/mnt/SDCARD/Emu/SEARCH/../../App/SearchFilter"
#define LAUNCH_PATH APP_ROOT "/launch.sh"

namespace tools {

void fixFavoritesBoxart(void)
{
    vector<GameEntry> favorites = loadGameEntries(FAVORITES_PATH);
    string contents = "";

    for (auto &entry : favorites) {
        if (entry.rompath.find(":") != string::npos) {
            vector<string> tokens = split(entry.rompath, ":");
            entry.launch = tokens[0];
            entry.rompath = tokens[1];
        }

        contents += entry.toJson() + "\n";
    }

    putFile(FAVORITES_PATH, contents);
}

void sortFavorites(void)
{
    vector<GameEntry> favorites = loadGameEntries(FAVORITES_PATH);
    string contents = "";

    sort(favorites.begin(), favorites.end(), [](GameEntry a, GameEntry b) {
        return a.label < b.label;
    });

    for (auto &entry : favorites)
        contents += entry.toJson() + "\n";

    putFile(FAVORITES_PATH, contents);
}

void addShourtcut(vector<GameEntry>* favorites, string label, string cmd)
{
    auto hasShortcut = [cmd](GameEntry entry){
        return entry.rompath == cmd;
    };
    if (any_of(favorites->begin(), favorites->end(), hasShortcut))
        return;
    favorites->push_back({label, LAUNCH_PATH, 5, APP_ROOT "/data/~Tools/" + cmd + ".miyoocmd"});
}

void addFavoritesTools(void)
{
    vector<GameEntry> favorites = loadGameEntries(FAVORITES_PATH);
    string contents = "";

    addShourtcut(&favorites, "~Fix boxart", "Fix favorites boxart");
    addShourtcut(&favorites, "~Sort alphabetically", "Sort favorites");

    for (auto &entry : favorites)
        contents += entry.toJson() + "\n";

    putFile(FAVORITES_PATH, contents);
}

void cleanRecentList(bool only_garbage)
{
    vector<GameEntry> recents = loadGameEntries(RECENTLIST_PATH);
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
