#if !defined(VERSION)
#define VERSION ""
#endif

#include <cstring>
#include <iostream>
#include <string>

using std::string;

#include "SDL/SDL.h"

#include "../common/display.hpp"
#include "keyboard.hpp"

extern "C" int keyboard(SDL_Surface *, SDL_Surface *, char *, char *, char *);

int main(int argc, char **argv)
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
    Display *display = new Display();
    Keyboard *kb = new Keyboard(display, initial_value, title);

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

int keyboard(SDL_Surface *video, SDL_Surface *screen, char *title, char *initial_value, char *out_value)
{
    int quit = 0;
    Display *display = new Display(video, screen);
    Keyboard *kb = new Keyboard(display, initial_value, title);

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
        strncpy(out_value, kb->getValue().c_str(), 256);
    }

    delete kb;

    return kb->cancelled;
}
