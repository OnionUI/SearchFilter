#if !defined(VERSION)
#define VERSION ""
#endif

#include <iostream>
#include <string>
#include <stdio.h>
using std::string;

#include "../common/display.hpp"
#include "../common/sysutils.hpp"
#include "../common/db_interface.hpp"
#include "../kbinput/keyboard.hpp"

#define ACTIVE_SEARCH "active_search"
#define DB_NAME "data"
#define DB_PATH "./" DB_NAME "/" DB_NAME "_cache2.db"

void performSearch(string keyword)
{
    if (!exists(DB_PATH) && !db_create(DB_PATH, DB_NAME)) {
        std::cerr << "Could't create database" << std::endl;
        return;
    }

    map<string, vector<RomEntry>> results = db_searchAll(keyword);

    for (auto &result : results) {
        int count = result.second.size();
        std::cout << result.first << ": " << count << std::endl;

        for (auto &item : result.second) {
            std::cout << item.disp << ", " << item.path << ", " << item.ppath << std::endl;
        }
    }
}

int main(int argc, char** argv)
{
    if (argc >= 2 && std::string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    Display* display = new Display();
    string keyword = getFile(ACTIVE_SEARCH);

    int ec = kbinput(display, "Search:", keyword, &keyword);

    if (ec == 0) {
        if (keyword.length() > 0) {
            putFile(ACTIVE_SEARCH, keyword);
            performSearch(keyword);
        }
        else {
            remove(ACTIVE_SEARCH);
            remove(DB_PATH);
        }
    }

    delete display;
    return 0;
}
