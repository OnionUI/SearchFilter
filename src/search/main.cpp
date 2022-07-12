#if !defined(VERSION)
#define VERSION ""
#endif

#include <iostream>
#include <string>
#include <stdio.h>
using std::string;

#include "../common/display.hpp"
#include "../common/utils.hpp"
#include "../common/config.hpp"
#include "../kbinput/keyboard.hpp"

#include "search.hpp"

int main(int argc, char** argv)
{
    if (argc >= 2 && string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    Display* display = new Display();
    string keyword = getFile(ACTIVE_SEARCH);
    int ec = 0;

    if (argc >= 2 && string(argv[1]) == "clear") keyword = "";
    else ec = kbinput(display, "Enter search term", keyword, &keyword);

    if (ec == 0) {
        search_icon = IMG_Load("res/icon_search.png");

        if (keyword.length() > 0) {
            putFile(ACTIVE_SEARCH, keyword);
            updateDisplay(display, "Searching...");
        }
        else {
            remove(ACTIVE_SEARCH);
        }
        performSearch(display, keyword);

        SDL_FreeSurface(search_icon);
        search_icon = NULL;
    }

    delete display;
    return 0;
}
