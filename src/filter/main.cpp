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

    if (mode == "refresh")
        refreshRoms(emu_path);
    else if (mode == "clear_filter")
        clearFilter(emu_path);
    else if (mode == "install_filter")
        installFilter();
    else if (mode == "uninstall_filter")
        uninstallFilter();
    else if (mode == "filter") {
        Display* display = new Display();
        applyFilter(display, emu_path);
        delete display;
    }

    return 0;
}
