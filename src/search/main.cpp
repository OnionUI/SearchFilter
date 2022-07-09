#if !defined(VERSION)
#define VERSION ""
#endif

#include <iostream>
#include <string>

using std::string;

int main(int argc, char** argv)
{
    if (argc >= 2 && std::string(argv[1]) == "--version") {
        std::cout << VERSION;
        exit(0);
    }

    return 0;
}
