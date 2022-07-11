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

    Display* display = new Display();
    string mode = argc >= 2 ? string(argv[1]) : "";

    if (mode == "nocache") {
        display->centerText("Go to the console's game list", {320, 220}, display->fonts.display);
        display->centerText("to cache it's games", {320, 270}, display->fonts.display);
        display->flip();
        sleep(2);
    }
    else if (mode == "boxart") {
        display->flipText("Fixing boxart...");
        tools::fixFavoritesBoxart();
        display->flipText("Boxart fixed");
        sleep(1);
    }
    else if (mode == "favsort") {
        display->flipText("Sorting favorites...");
        tools::sortFavorites();
        display->flipText("Sorted");
        sleep(1);
    }
    else if (mode == "favtools") {
        display->flipText("Adding tools to favorites...");
        tools::addFavoritesTools();
        display->flipText("Tools added");
        sleep(1);
    }

    delete display;
    return 0;
}
