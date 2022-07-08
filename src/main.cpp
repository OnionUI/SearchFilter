#if !defined(VERSION)
#define VERSION "0.1-alpha"
#endif

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
using std::string;
using std::map;
using std::vector;

#include "common/display.hpp"
#include "common/resource.hpp"
#include "common/legend.hpp"
#include "common/keymap.hpp"
#include "common/sysutils.hpp"
#include "common/db_interface.hpp"
#include "kbinput/keyboard.hpp"

#define ACTIVE_KEYWORD "active_keyword"

int kbinput(Display* display, string title, string value, string* output)
{
    bool quit = false;
    Keyboard* kb = new Keyboard(display, value, title);

    auto input_handler = [&kb](SDLKey key, Uint8 type, int repeating) {
        return kb->handleKeyPress(key, type, repeating);
    };

    auto frame_handler = [display, input_handler](void) {
        return display->onInputEvent(input_handler);
    };
    
    while (!quit) {
        quit = display->requestFrame(frame_handler);
    }

    bool cancelled = kb->cancelled;

    if (!cancelled)
        value = kb->getValue();

    delete kb;

    *output = value;
    return cancelled;
}

map<string, int> prompt_keyword(Display* display, string* keyword)
{
    map<string, int> counts;
    string prev_keyword = *keyword;
    string temp_keyword = "";
    int ec = kbinput(display, "Enter keyword", prev_keyword, &temp_keyword);
    int length = prev_keyword.length();

    if (ec == 0 && temp_keyword != prev_keyword) {
        *keyword = temp_keyword;
        length = keyword->length();

        if (length > 0) {
            display->clear();
            display->centerText("Please wait...", {320, 240});
            display->flip();
            counts = db_filterAll(temp_keyword);
            putFile(ACTIVE_KEYWORD, temp_keyword);
        }
    }

    if (ec != 0 && length == 0)
        exit(0);

    return counts;
};

int main(int argc, char *argv[])
{
    if (argc >= 2 && std::string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    Display* display = new Display();
    string keyword = getFile(ACTIVE_KEYWORD);
    map<string, int>* counts = new map<string, int>();

    auto updateCounts = [&counts](map<string, int> _counts) {
        if (_counts.empty())
            return;
        counts->clear();
        counts->swap(_counts);
    };

    if (keyword.length() == 0) {
        updateCounts(prompt_keyword(display, &keyword));
    }
    else {
        updateCounts(db_filterAll(keyword));
    }

    bool quit = false;
    bool changed = true;

    ResourceMap res = loadResources({
        {RES_BUTTON_A},
        {RES_BUTTON_B},
        {RES_BUTTON_X},
    });

    Legend legend_off = Legend(display, new std::vector<LegendItem>{
        {res[RES_BUTTON_A], "Enter", "A"},
        {res[RES_BUTTON_B], "Back", "B"}
    });
    Legend legend_on = Legend(display, new std::vector<LegendItem>{
        {res[RES_BUTTON_A], "Enter", "A"},
        {res[RES_BUTTON_B], "Back", "B"}
    }, new std::vector<LegendItem>{
        {res[RES_BUTTON_X], "Clear", "X"}
    });

    auto input_handler = [display, updateCounts, &changed, &keyword](SDLKey key, Uint8 type, int repeating) {
        if (type != SDL_KEYUP)
            return false;

        switch (key)
        {
        case BUTTON_A:
            updateCounts(prompt_keyword(display, &keyword));
            changed = true;
            break;
        case BUTTON_X:
            keyword = "";
            changed = true;
            break;
        case BUTTON_MENU:
        case BUTTON_B:
            return true;
        default: break;
        }

        return false;
    };

    auto render = [display, counts, &legend_on, &legend_off, &changed, &keyword](void) {
        if (!changed)
            return;
            
        display->clear();
        if (keyword.length()) {
            display->centerText("'" + keyword + "'", {320, 70}, display->fonts.display);
            display->centerText("Press A to edit or X to clear", {320, 120});

            if (!counts->empty()) {
                bool has_missing = false;
                int total = 0;
                vector<string> lines;
                stringstream ss;

                auto addItem = [display, &lines, &ss](string name, int count) {
                    string curr = count == -1 ? name + "*" : name + ": " + std::to_string(count);

                    if (display->textWidth(ss.str() + curr) > 600) {
                        lines.push_back(ss.str());
                        ss.str("");
                    }

                    if (ss.str().length() > 0)
                        ss << "   ";

                    ss << curr;
                };

                for (auto &item : *counts) {
                    if (item.second > 0)
                        total += item.second;
                    else if (!has_missing && item.second == -1)
                        has_missing = true;
                    else if (item.second == -2)
                        continue;
                    addItem(item.first, item.second);
                }

                addItem("Total", total);

                if (ss.str().length())
                    lines.push_back(ss.str());

                int offset = 0;
                for (auto &line : lines) {
                    display->centerText(line, {320, 210 + offset});
                    offset += 40;
                }

                if (has_missing)
                    display->text("*Not cached", {20, 380}, NULL, {160, 80, 80});
            }

            legend_on.render();
        }
        else {
            display->centerText("Press A to apply filter", {320, 120});
            legend_off.render();
        }
        display->flip();

        if (keyword.length() == 0) {
            db_clearAll();
            remove(ACTIVE_KEYWORD);
        }

        changed = false;
    };

    auto frame_handler = [display, input_handler, render](void) {
        bool quit = display->onInputEvent(input_handler);
        render();
        return quit;  
    };

    while (!quit) {
        quit = display->requestFrame(frame_handler);
    }

    delete counts;
    delete display;
    return 0;
}
