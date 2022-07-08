#if !defined(VERSION)
#define VERSION "0.1-alpha"
#endif

#include <iostream>
#include <string>
#include <cstring>

#include "SDL/SDL.h"

#include "../common/display.hpp"
#include "keyboard.hpp"

int main(int argc, char** argv)
{
    if (argc >= 2 && std::string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    std::string initial_value = "";
    std::string title = "";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            std::cerr << "Usage: kbinput -i [INITIAL VALUE] -t [SCREEN TITLE]" << std::endl;
            exit(0);
        }
        else if (strcmp(argv[i], "-i") == 0)
            initial_value = (std::string)(argv[++i]);
        else if (strcmp(argv[i], "-t") == 0)
            title = (std::string)(argv[++i]);
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

    if (!kb->cancelled)
        std::cout << kb->getValue();

    delete kb;
    delete display;

    return kb->cancelled;
}
