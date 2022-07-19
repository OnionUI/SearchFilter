#if !defined(GAMELIST_HPP__)
#define GAMELIST_HPP__

#include <string>
#include <vector>
#include <pugixml/src/pugixml.hpp>

using std::string;
using std::vector;
using pugi::xml_document;
using pugi::xml_parse_result;
using pugi::xml_node;

#include "utils.hpp"

struct GamelistEntry
{
    string path = "";
    string name = "";
    string desc = "";
    float rating = -1.0;
    string releasedate = "";
    string developer = "";
    string publisher = "";
    string genre = "";
    string players = "";
    string hash = "";
    int genreid = -1;
};

vector<GamelistEntry> loadGamelist(string path)
{
    vector<GamelistEntry> entries;
    xml_document doc;
    xml_parse_result result = doc.load_file(path.c_str());
    
    if (!result)
        return entries;
        
    for (xml_node game : doc.child("gameList").children("game")) {
        GamelistEntry entry = {
            .path = game.child("path").text().as_string()
        };
        entries.push_back(entry);
    }
}


#endif // GAMELIST_HPP__
