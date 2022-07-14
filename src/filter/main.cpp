#if !defined(VERSION)
#define VERSION ""
#endif

#include <iostream>
#include <stdio.h>
#include <string>
using std::string;

#include "../common/display.hpp"
#include "filter.hpp"

int main(int argc, char *argv[])
{
    if (argc >= 2 && string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    string mode = argc >= 2 ? string(argv[1]) : "";
    string emu_path = argc >= 3 ? string(argv[2]) : "";

    if (mode == "refresh") {
        refreshRoms(emu_path);
        return 0;
    }
    else if (mode == "clear_filter") {
        clearFilter(emu_path);
        return 0;
    }

    Display* display = new Display();
    string message = "";

    if (mode == "install_filter") {
        display->flipText("Installing filter...");
        installFilter();
        message = "Filter installed";
    }
    else if (mode == "uninstall_filter") {
        display->flipText("Uninstalling filter...");
        uninstallFilter();
        message = "Filter uninstalled";
    }
    else if (mode == "filter") {
        applyFilter(display, emu_path);
    }

    if (message.length() > 0) {
        display->flipText(message);
        sleep(1);
    }

    delete display;
    return 0;
}
