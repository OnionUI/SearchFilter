#if !defined(FIX_BOXART_HPP__)
#define FIX_BOXART_HPP__

#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <json/json.h>

using std::sort;
using std::any_of;
using std::string;
using std::to_string;
using std::getline;
using std::ifstream;
using std::vector;

#include "../common/utils.hpp"

#define RECENTLIST_PATH "/mnt/SDCARD/Roms/recentlist.json"
#define FAVORITES_PATH "/mnt/SDCARD/Roms/favourite.json"
#define APP_ROOT "/mnt/SDCARD/Emu/SEARCH/../../App/SearchFilter"
#define LAUNCH_PATH APP_ROOT "/launch.sh"

namespace tools {

struct GameEntry
{
    string label = "";
    string launch = "";
    int type = 5;
    string rompath = "";
    string imgpath = "";

    static GameEntry fromJson(string json_str)
    {
        GameEntry entry;
        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(json_str, root)) {
            std::cerr << "Error parsing the string" << std::endl;
            return entry;
        }

        auto addString = [root](string key, string* dst) {
            if (root.isMember(key))
                *dst = root[key].asString();
        };

        auto addInt = [root](string key, int* dst) {
            if (root.isMember(key))
                *dst = root[key].asInt();
        };

        addString("label", &entry.label);
        addString("launch", &entry.launch);
        addInt("type", &entry.type);
        addString("rompath", &entry.rompath);
        addString("imgpath", &entry.imgpath);

        return entry;
    }

    string toJson(void)
    {
        string json_str = "{";
        json_str += "\"label\":\"" + label + "\",";
        json_str += "\"launch\":\"" + launch + "\",";
        json_str += "\"type\":" + to_string(type) + ",";
        if (imgpath.length() > 0)
            json_str += "\"imgpath\":\"" + imgpath + "\",";
        json_str += "\"rompath\":\"" + rompath + "\"";
        json_str += "}";
        return json_str;
    }
};

vector<GameEntry> loadGameEntries(string json_path) {
    vector<GameEntry> entries;
    
    if (!exists(json_path))
        return entries;

    ifstream infile(json_path);

    string line;
    while (getline(infile, line)) {
        GameEntry entry = GameEntry::fromJson(line);
        if (entry.label.length() == 0)
            continue;
        entries.push_back(entry);
    }

    infile.close();

    return entries;
}

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
    favorites->push_back({label, LAUNCH_PATH, 5, APP_ROOT "/data/~Tools/" + cmd + ".txt"});
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
        else if (entry.launch == LAUNCH_PATH || (!only_garbage && entry.type == 3)) {
            continue;
        }
        contents += entry.toJson() + "\n";
    }

    putFile(RECENTLIST_PATH, contents);
}

} // namespace tools

#endif // FIX_BOXART_HPP__
