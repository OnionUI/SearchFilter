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
    int id = -1;
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

    static ConfigEmu load(string json_path, int _id)
    {
        ConfigEmu config;
        Json::Value root;

        config.id = _id;

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

    static ConfigEmu load(string json_path)
    {
        return ConfigEmu::load(json_path, -1);
    }

    string toJson(bool new_lines = true)
    {
        string json_str = "{";

        auto addLine = [&json_str, new_lines](string key, string value,
                                              bool is_number = false,
                                              bool remove_empty = false,
                                              bool is_last = false) {
            if (value.length() == 0 && remove_empty)
                return;
            json_str += wrapQuotes(key) + ":" + (is_number ? value : wrapQuotes(value));
            if (!is_last) json_str += ",";
            if (new_lines) json_str += "\n";
        };

        addLine("label", label);
        addLine("icon", icon);
        addLine("iconsel", iconsel, false, true);
        addLine("launch", launch);
        addLine("rompath", rompath);
        addLine("imgpath", imgpath);
        addLine("gamelist", gamelist, false, true);
        addLine("useswap", to_string(useswap), true);
        addLine("shortname", to_string(shortname), true);
        addLine("hidebios", to_string(hidebios), true);
        addLine("extlist", extlist, false, false, true);

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
    int index = 0;

    subdirForEach(EMU_PATH, [&configs, &index](string name) {
        string config_path = CONFIG_PATH(name);

        if (name == IGNORED_EMU)
            return;

        if (!exists(config_path))
            return;

        configs.push_back(ConfigEmu::load(config_path, index++));
    });

    return configs;
}

ConfigEmu getEmulatorConfig(string emu_path)
{
    ConfigEmu config;
    string config_path = emu_path + "/config.json";

    if (!exists(config_path))
        return config;

    return ConfigEmu::load(config_path);
}

#endif // CONFIG_HPP__
