#if !defined(VERSION)
#define VERSION ""
#endif

#include <iostream>
#include <string>
#include <cstring>

using std::string;

#include "SDL/SDL.h"

#include "../common/display.hpp"
#include "keyboard.hpp"

int main(int argc, char** argv)
{
    if (argc >= 2 && string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    string initial_value = "";
    string title = "";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            std::cerr << "Usage: kbinput -i [INITIAL VALUE] -t [SCREEN TITLE]" << std::endl;
            exit(0);
        }
        else if (strcmp(argv[i], "-i") == 0)
            initial_value = (string)(argv[++i]);
        else if (strcmp(argv[i], "-t") == 0)
            title = (string)(argv[++i]);
        else {
            std::cerr << "Unrecognized argument: " << argv[i] << std::endl;
            exit(0);
        }
    }

    int quit = 0;
    Display* display = new Display();
    Keyboard* kb = new Keyboard(display, initial_value, title);

    auto input_handler = [&kb](SDLKey key, Uint8 type, int repeating) {
        return kb->handleKeyPress(key, type, repeating);
    };

    auto frame_handler = [display, input_handler](void) {
        return display->onInputEvent(input_handler);
    };

    while (!quit) {
        quit = display->requestFrame(frame_handler);
    }

    delete display;

    if (!kb->cancelled) {
        std::cout << "\n\nRESULT:" << std::endl;
        std::cout << kb->getValue() << std::endl;
    }
    
    delete kb;

    return kb->cancelled;
}
