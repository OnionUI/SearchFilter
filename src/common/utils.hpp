#if !defined(UTILS_HPP__)
#define UTILS_HPP__

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <dirent.h>

using std::ifstream;
using std::ofstream;
using std::ios;
using std::string;
using std::vector;
using std::map;
using std::function;

#define IGNORE_HIDDEN_FILES 1

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

vector<string> split(const string &s, string delim)
{
    size_t last = 0;
    size_t next = 0;
    vector<string> tokens;
    
    while ((next = s.find(delim, last)) != string::npos) {
        tokens.push_back(s.substr(last, next - last));
        last = next + delim.length();
    }

    if (last < s.length())
        tokens.push_back(s.substr(last));

    return tokens;
}

string join(const vector<string> &ss, string delim = "")
{
    string s = "";
    bool first = true;
    for (auto &str : ss) {
        if (first) first = false;
        else s += delim;
        s += str;
    }
    return s;
}

template<typename T>
vector<T> removeValue(const vector<T> &vec, const T &value)
{
    vector<T> res;
    for (auto &item : vec) {
        if (item != value)
            res.push_back(item);
    }
    return res;
}

string replaceAll(const string &s, string x, string y)
{
    string buffer = "";
    vector<string> tokens = split(s, x);

    int count = tokens.size();
    for (int i = 0; i < count; i++) {
        buffer += string(tokens[i]);
        if (i < count - 1)
            buffer += y;
    }

    return buffer;
}

string escape(const string &s)
{
    string _s = replaceAll(s, "\\", "\\\\");
    return replaceAll(_s, "'", "\\'");
}

const string WHITESPACE = " \n\r\t\f\v";

string ltrim(const string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == string::npos) ? "" : s.substr(start);
}
 
string rtrim(const string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == string::npos) ? "" : s.substr(0, end + 1);
}
 
string trim(const string &s) {
    return rtrim(ltrim(s));
}

string wrapQuotes(const string &s, const string quote_char = "\"") {
    return quote_char + s + quote_char;
}

int findNth(const std::string& str, const std::string& findMe, int nth)
{
    size_t  pos = 0;
    int     cnt = 0;

    while( cnt != nth )
    {
        pos+=1;
        pos = str.find(findMe, pos);
        if ( pos == std::string::npos )
            return -1;
        cnt++;
    }
    return pos;
}

string tolower(const string &s)
{
    string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return result;
}

string dirname(string path)
{
    int pos = path.find_last_of("/");
    return pos == -1 ? "" : path.substr(0, pos);
}

string basename(string path)
{
    return path.substr(path.find_last_of("/") + 1);
}

string getExtension(string path)
{
    int pos = path.find_last_of(".");
    if (pos != string::npos)
        return path.substr(pos + 1);
    return "";
}

string removeExtension(string path)
{
    int pos = path.find_last_of(".");
    if (pos != string::npos)
        return path.substr(0, pos);
    return path;
}

string fullpath(string root, string rel = "")
{
    string fullpath = "";
    string cmd;
    if (rel.length() > 0)
        cmd = "cd '" + root + "' >/dev/null 2>&1 && cd '" + rel + "' >/dev/null 2>&1 && pwd -P";
    else
        cmd = "cd '" + root + "' >/dev/null 2>&1 && pwd -P";
    if (exec(cmd, &fullpath))
        return "";
    return fullpath;
}

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
    string dir_name = dirname(file_path);
    if (dir_name.length() > 0)
        system(string("mkdir -p '" + dir_name + "'").c_str());
    ofstream file(file_path, ios::trunc);
    if (file.is_open()) {
        file << contents;
        file.close();
    }
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

void subdirForEach(string path, function<void(string)> callback) {
    DIR* dirFile = opendir(path.c_str());

    if (!dirFile)
        return;

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

        callback(name);
    }

    closedir(dirFile);
}

#endif // UTILS_HPP__
