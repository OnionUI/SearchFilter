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

#define FAVORITES_PATH "/mnt/SDCARD/Roms/favourite.json"
#define FAV_TOOL_LAUNCH "/mnt/SDCARD/Emu/SEARCH/../../App/SearchFilter/launch.sh"

namespace tools {

struct FavoriteEntry
{
    string label = "";
    string launch = "";
    int type = 5;
    string rompath = "";

    static FavoriteEntry fromJson(string json_str)
    {
        FavoriteEntry entry;
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

        return entry;
    }

    string toJson(void)
    {
        string json_str = "{";
        json_str += "\"label\":\"" + label + "\",";
        json_str += "\"launch\":\"" + launch + "\",";
        json_str += "\"type\":" + to_string(type) + ",";
        json_str += "\"rompath\":\"" + rompath + "\"";
        json_str += "}";
        return json_str;
    }
};

vector<FavoriteEntry> loadFavorites(void) {
    vector<FavoriteEntry> favorites;
    
    if (!exists(FAVORITES_PATH))
        return favorites;

    ifstream infile(FAVORITES_PATH);

    string line;
    while (getline(infile, line)) {
        FavoriteEntry entry = FavoriteEntry::fromJson(line);
        if (entry.label.length() == 0)
            continue;
        favorites.push_back(entry);
    }

    infile.close();

    return favorites;
}

void fixFavoritesBoxart(void)
{
    vector<FavoriteEntry> favorites = loadFavorites();
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
    vector<FavoriteEntry> favorites = loadFavorites();
    string contents = "";

    sort(favorites.begin(), favorites.end(), [](FavoriteEntry a, FavoriteEntry b) {
        return a.label < b.label;
    });

    for (auto &entry : favorites)
        contents += entry.toJson() + "\n";

    putFile(FAVORITES_PATH, contents);
}

void addShourtcut(vector<FavoriteEntry>* favorites, string label, string cmd)
{
    auto hasShortcut = [cmd](FavoriteEntry entry){
        return entry.rompath == cmd;
    };
    if (any_of(favorites->begin(), favorites->end(), hasShortcut))
        return;
    favorites->push_back({label, FAV_TOOL_LAUNCH, 5, cmd});
}

void addFavoritesTools(void)
{
    vector<FavoriteEntry> favorites = loadFavorites();
    string contents = "";
    string launch_path = fullpath(".") + "/launch.sh";

    addShourtcut(&favorites, "~Fix boxart", "boxart");
    addShourtcut(&favorites, "~Sort alphabetically", "favsort");

    for (auto &entry : favorites)
        contents += entry.toJson() + "\n";

    putFile(FAVORITES_PATH, contents);
}

} // namespace tools

#endif // FIX_BOXART_HPP__
