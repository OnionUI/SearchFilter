#if !defined(CONFIG_HPP__)
#define CONFIG_HPP__

#include <json/json.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using std::string;
using std::vector;

#include "sysutils.hpp"

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
    int useswap = 0;
    int shortname = 0;
    int hidebios = 0;
    string extlist = "";
};

ConfigEmu readEmuConfig(string config_path)
{
    ConfigEmu config;
    Json::Value root;
    std::ifstream ifs;

    ifs.open(config_path);

    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    config.path = dirname(config_path);

    if (!parseFromStream(builder, ifs, &root, &errs)) {
        std::cout << errs << std::endl;
        return config;
    }

    auto addString = [&config, root](string key, string* dst) {
        if (root.isMember(key))
            *dst = root[key].asString();
    };

    auto addInt = [&config, root](string key, int* dst) {
        if (root.isMember(key))
            *dst = root[key].asInt();
    };

    addString("label", &config.label);
    addString("icon", &config.icon);
    addString("iconsel", &config.iconsel);
    addString("launch", &config.launch);
    addString("rompath", &config.rompath);
    addString("imgpath", &config.imgpath);
    addInt("useswap", &config.useswap);
    addInt("shortname", &config.shortname);
    addInt("hidebios", &config.hidebios);
    addString("extlist", &config.extlist);

    return config;
}

vector<ConfigEmu> getEmulatorConfigs(void)
{
    vector<ConfigEmu> configs;

    subdirForEach(EMU_PATH, [&configs](string name) {
        string config_path = CONFIG_PATH(name);

        if (name == IGNORED_EMU)
            return;

        if (!exists(config_path))
            return;

        configs.push_back(readEmuConfig(config_path));
    });

    return configs;
}

#endif // CONFIG_HPP__
