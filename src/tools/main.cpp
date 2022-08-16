#if !defined(VERSION)
#define VERSION ""
#endif

#include <iostream>
#include <string>
#include <unistd.h>
using std::string;

#include "../common/display.hpp"

#include "tools.hpp"

int main(int argc, char** argv)
{
    bool show_display = false;
    bool clean_all = false;
    string mode = "unknown";
    
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--version") {
            std::cout << VERSION;
            exit(0);
        }
        else if (string(argv[i]) == "--display") {
            show_display = true;
        }
        else if (string(argv[i]) == "--clean_all") {
            clean_all = true;
        }
        else if (mode == "unknown") {
            mode = argv[i];
        }
        else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            exit(1);
        }
    }

    Display* display = show_display ? new Display() : NULL;
    auto flipText = [display](string text) {
        if (display)
            display->flipText(text);
    };

    if (mode == "boxart") {
        tools::fixFavoritesBoxart();
        flipText("Boxart fixed");
        sleep(1);
    }
    else if (mode == "favsort") {
        tools::sortFavorites();
        flipText("Favorites sorted");
        sleep(1);
    }
    else if (mode == "favsort2") {
        tools::sortFavoritesBySystem();
        flipText("Favorites sorted");
        sleep(1);
    }
    else if (mode == "favtools") {
        tools::addFavoritesTools();
        flipText("Shortcuts added");
        sleep(1);
    }
    else if (mode == "recents") {
        tools::cleanRecentList(RECENTLIST_PATH, clean_all);
        tools::cleanRecentList(RECENTLIST_HIDDEN_PATH, clean_all);
        flipText("Recent list cleaned");
        sleep(1);
    }

    delete display;
    return 0;
}
