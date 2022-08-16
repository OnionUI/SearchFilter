#if !defined(GAME_ENTRY_HPP__)
#define GAME_ENTRY_HPP__

#include <string>
#include <fstream>
#include <vector>
#include <json/json.h>

using std::string;
using std::to_string;
using std::getline;
using std::ifstream;
using std::vector;

#include "utils.hpp"

#define RECENTLIST_PATH "/mnt/SDCARD/Roms/recentlist.json"
#define RECENTLIST_HIDDEN_PATH "/mnt/SDCARD/Roms/recentlist-hidden.json"
#define FAVORITES_PATH "/mnt/SDCARD/Roms/favourite.json"

struct GameJsonEntry
{
    string label = "";
    string launch = "";
    int type = 5;
    string rompath = "";
    string imgpath = "";
    string emupath = "";

    static GameJsonEntry fromJson(string json_str)
    {
        GameJsonEntry entry;
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

        if (entry.rompath.find(":") != string::npos) {
            vector<string> tokens = split(entry.rompath, ":");
            entry.launch = tokens[0];
            entry.rompath = tokens[1];
        }

        entry.emupath = split(entry.rompath, "/../../")[0];

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

vector<GameJsonEntry> loadGameJsonEntries(string json_path) {
    vector<GameJsonEntry> entries;
    
    if (!exists(json_path))
        return entries;

    ifstream infile(json_path);

    string line;
    while (getline(infile, line)) {
        GameJsonEntry entry = GameJsonEntry::fromJson(line);
        if (entry.label.length() == 0)
            continue;
        entries.push_back(entry);
    }

    infile.close();

    return entries;
}

#endif // GAME_ENTRY_HPP__
