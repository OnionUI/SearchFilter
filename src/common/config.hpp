#if !defined(CONFIG_HPP__)
#define CONFIG_HPP__

#include <json/json.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;
using std::ifstream;

#include "utils.hpp"

#define IGNORED_EMU "SEARCH"
#define EMU_PATH "/mnt/SDCARD/Emu"
#define CONFIG_PATH(name) EMU_PATH "/" + name + "/config.json"

struct ConfigEmu
{
    string path = "";
    string label = "";
    string icon = "";
    string iconsel = "";
    string launch = "";
    string rompath = "";
    string imgpath = "";
    string gamelist = "";
    int useswap = 0;
    int shortname = 0;
    int hidebios = 0;
    string extlist = "";

    static ConfigEmu load(string json_path)
    {
        ConfigEmu config;
        Json::Value root;

        if (!exists(json_path))
            return config;
        
        ifstream ifs;
        ifs.open(json_path);

        Json::CharReaderBuilder builder;
        JSONCPP_STRING errs;

        config.path = json_path;

        if (!parseFromStream(builder, ifs, &root, &errs)) {
            std::cerr << errs << std::endl;
            return config;
        }

        ifs.close();

        auto addString = [root](string key, string* dst) {
            if (root.isMember(key))
                *dst = root[key].asString();
        };

        auto addInt = [root](string key, int* dst) {
            if (root.isMember(key))
                *dst = root[key].asInt();
        };

        addString("label", &config.label);
        addString("icon", &config.icon);
        addString("iconsel", &config.iconsel);
        addString("launch", &config.launch);
        addString("rompath", &config.rompath);
        addString("imgpath", &config.imgpath);
        addString("gamelist", &config.gamelist);
        addInt("useswap", &config.useswap);
        addInt("shortname", &config.shortname);
        addInt("hidebios", &config.hidebios);
        addString("extlist", &config.extlist);

        return config;
    }

    string toJson(void)
    {
        string json_str = "{";
        json_str += "\"label\":\"" + label + "\",";
        json_str += "\"icon\":\"" + icon + "\",";
        json_str += "\"iconsel\":\"" + iconsel + "\",";
        json_str += "\"launch\":\"" + launch + "\",";
        json_str += "\"rompath\":\"" + rompath + "\",";
        json_str += "\"imgpath\":\"" + imgpath + "\",";
        json_str += "\"gamelist\":\"" + gamelist + "\",";
        json_str += "\"useswap\":" + to_string(useswap) + ",";
        json_str += "\"shortname\":" + to_string(shortname) + ",";
        json_str += "\"hidebios\":" + to_string(hidebios) + ",";
        json_str += "\"extlist\":\"" + extlist + "\"";
        json_str += "}";
        return json_str;
    }

    void save(void)
    {
        putFile(path, toJson());
    }
};

vector<ConfigEmu> getEmulatorConfigs(void)
{
    vector<ConfigEmu> configs;

    subdirForEach(EMU_PATH, [&configs](string name) {
        string config_path = CONFIG_PATH(name);

        if (name == IGNORED_EMU)
            return;

        if (!exists(config_path))
            return;

        configs.push_back(ConfigEmu::load(config_path));
    });

    return configs;
}

#endif // CONFIG_HPP__
