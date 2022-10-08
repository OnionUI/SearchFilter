#if !defined(VERSION)
#define VERSION ""
#endif

#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>

using std::string;
using std::vector;

#include "../common/display.hpp"
#include "../common/utils.hpp"
#include "../common/ConfigEmu.hpp"
#include "../kbinput/keyboard.hpp"

#include "search.hpp"

#define EMU_DIR "/mnt/SDCARD/Emu/SEARCH"
#define EMU_CONFIG_DST EMU_DIR "/config.json"
#define EMU_CONFIG_SRC "/mnt/SDCARD/.tmp_update/res/search_config.json"

int main(int argc, char** argv)
{
    if (argc >= 2 && string(argv[1]) == "--version") {
        std::cout << VERSION;
        return 0;
    }

    if (argc >= 3 && string(argv[1]) == "setstate") {
        string label = string(argv[2]);
        std::cout << "label = " << label << std::endl;
        vector<string> state_json = split(getFile("/tmp/state.json"), "]");
        stringstream ss;
        ss << state_json[0];
        ss << ",{\"title\":-1,\"type\":5,\"currpos\":0,\"pagestart\":0,\"pageend\":5,\"emuname\":\"" << label << "\"}]}";
        std::cout << "state = " << ss.str() << std::endl;
        putFile("/tmp/state.json", ss.str());
        return 0;
    }

    Display* display = new Display();
    string keyword = getFile(ACTIVE_SEARCH);
    int ec = 0;

    if (argc >= 2 && string(argv[1]) == "clear") keyword = "";
    else ec = kbinput(display, "Enter search term", keyword, &keyword);

    if (ec == 0) {
        search_icon = IMG_Load("/mnt/SDCARD/Icons/app/search.png");

        if (keyword.length() > 0) {
            putFile(ACTIVE_SEARCH, keyword);
            if (!exists(EMU_CONFIG_DST)) {
                system("mkdir -p " EMU_DIR);
                copyFile(EMU_CONFIG_SRC, EMU_CONFIG_DST);
            }
            if (!exists("res/state-backup.json"))
                copyFile("/tmp/state.json", "res/state-backup.json");
            copyFile("res/search_state.json", "/tmp/state.json");
        }
        else {
            remove(ACTIVE_SEARCH);
            if (!exists("config/.keepSearch")) {
                if (exists(EMU_CONFIG_DST))
                    remove(EMU_CONFIG_DST);
                copyFile("res/state-backup.json", "/tmp/state.json");
                remove("res/state-backup.json");
            }
        }

        performSearch(display, keyword);

        SDL_FreeSurface(search_icon);
        search_icon = NULL;
    }

    delete display;
    return 0;
}
