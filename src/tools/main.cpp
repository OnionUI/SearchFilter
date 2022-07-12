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
    if (argc >= 2 && string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    bool no_display = argc >= 3 && string(argv[2]) == "--silent";

    Display* display = no_display ? NULL : new Display();
    auto flipText = [display](string text) {
        if (display)
            display->flipText(text);
    };

    string mode = argc >= 2 ? string(argv[1]) : "";

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
    else if (mode == "favtools") {
        tools::addFavoritesTools();
        flipText("Shortcuts added");
        sleep(1);
    }
    else if (mode == "recents") {
        tools::cleanRecentList(no_display);
        flipText("Recent list cleaned");
        sleep(1);
    }

    delete display;
    return 0;
}
