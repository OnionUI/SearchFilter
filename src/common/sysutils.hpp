#if !defined(SYSUTILS_HPP__)
#define SYSUTILS_HPP__

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <functional>
#include <unistd.h>
#include <dirent.h>

using std::ifstream;
using std::ofstream;
using std::ios;
using std::string;
using std::map;
using std::function;

#define IGNORE_HIDDEN_FILES 1

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

bool dirEmpty(string path) {
    DIR* dirFile = opendir(path.c_str());

    if (!dirFile)
        return false;

    struct dirent* item;
    errno = 0;

    while ((item = readdir(dirFile)) != NULL) {
        string name(item->d_name);

        // Skip navigation dirs
        if (name == "." || name == "..") continue;

        // Ignore hidden directories
        if (IGNORE_HIDDEN_FILES && (name[0] == '.')) continue;

        closedir(dirFile);
        return false;
    }

    closedir(dirFile);
    return true;
}

template<typename T>
map<string, T> subdirForEach(string path, function<T(string)> callback) {
    map<string, T> values;
    DIR* dirFile = opendir(path.c_str());

    if (!dirFile)
        return values;

    struct dirent* item;
    errno = 0;

    while ((item = readdir(dirFile)) != NULL) {
        string name(item->d_name);

        // Skip files
        if (item->d_type != DT_DIR) continue;
        // Skip navigation dirs
        if (name == "." || name == "..") continue;

        // Ignore hidden directories
        if (IGNORE_HIDDEN_FILES && (name[0] == '.')) continue;

        // Ignore empty directories
        if (dirEmpty(path + "/" + name)) continue;

        values[name] = callback(name);
    }

    closedir(dirFile);
    return values;
}

#endif // SYSUTILS_HPP__
