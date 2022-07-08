#if !defined(SYSUTILS_HPP__)
#define SYSUTILS_HPP__

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

using std::ifstream;
using std::ofstream;
using std::ios;
using std::string;

bool exists(string file_path)
{
    return access(file_path.c_str(), F_OK) == 0;
}

void copyFile(string src_path, string dst_path)
{
    ifstream src(src_path, ios::binary);
    ofstream dst(dst_path, ios::binary);
    if (src.is_open() && dst.is_open()) {
        dst << src.rdbuf();
        src.close();
        dst.close();
    }
}

string getFile(string file_path)
{
    string contents = "";
    string line;
    ifstream file(file_path);
    if (file.is_open()) {
        while (std::getline(file, line)) {
            contents += line + '\n';
        }
        file.close();
    }
    int last = contents.length() - 1;
    if (last >= 0 && contents[last] == '\n')
        contents.erase(last);
    return contents;
}

void putFile(string file_path, string contents)
{
    ofstream file(file_path, ios::trunc);
    if (file.is_open()) {
        file << contents;
        file.close();
    }
}

int exec(string command, string* stdout)
{
    char buffer[128];
    *stdout = "";

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "popen failed!" << std::endl;
        exit(-1);
    }

    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            *stdout += buffer;
    }

    int last = stdout->length() - 1;
    if (last >= 0 && stdout->at(last) == '\n')
        stdout->erase(last);

    return WEXITSTATUS(pclose(pipe));
}

#endif // SYSUTILS_HPP__
