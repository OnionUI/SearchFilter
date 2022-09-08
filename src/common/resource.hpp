#if !defined(HPP_RESOURCE)
#define HPP_RESOURCE

#include <string>
#include <map>
#include <vector>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#define RES_BASE "res/"

#define RES_KEY_BACKSPACE "kbinput/key_backspace"
#define RES_KEY_SHIFT "kbinput/key_shift"
#define RES_KEY_SPACE "kbinput/key_space"
#define RES_KEY_CURSOR_LEFT "kbinput/key_cursor_left"
#define RES_KEY_CURSOR_RIGHT "kbinput/key_cursor_right"

#define RES_SMALL_A "kbinput/small_a"
#define RES_SMALL_B "kbinput/small_b"
#define RES_SMALL_X "kbinput/small_x"
#define RES_SMALL_Y "kbinput/small_y"
#define RES_SMALL_L "kbinput/small_l"
#define RES_SMALL_R "kbinput/small_r"
#define RES_SMALL_L2 "kbinput/small_l2"
#define RES_SMALL_R2 "kbinput/small_r2"
#define RES_SMALL_START "kbinput/small_start"
#define RES_SMALL_SELECT "kbinput/small_select"

#define RES_BUTTON_A "kbinput/button_a"
#define RES_BUTTON_B "kbinput/button_b"
#define RES_BUTTON_X "kbinput/button_x"
#define RES_BUTTON_Y "kbinput/button_y"
#define RES_BUTTON_L "kbinput/button_l"
#define RES_BUTTON_R "kbinput/button_r"
#define RES_BUTTON_L2 "kbinput/button_l2"
#define RES_BUTTON_R2 "kbinput/button_r2"
#define RES_BUTTON_START "kbinput/small_start"
#define RES_BUTTON_SELECT "kbinput/small_select"

typedef std::map<std::string, SDL_Surface*> ResourceMap;
struct ResourceName { std::string name = ""; std::string ext = "png"; };

ResourceMap loadResources(std::string base_path, std::vector<ResourceName> files)
{
    ResourceMap resource_map = ResourceMap();

    for (auto &file : files) {
        std::string path = base_path + file.name + "." + file.ext;
        resource_map[file.name] = IMG_Load(path.c_str());
    }

    return resource_map;
}
ResourceMap loadResources(std::vector<ResourceName> files)
{
    return loadResources(RES_BASE, files);
}

void freeResources(ResourceMap* resource_map)
{
    for (auto &item : *resource_map) {
        SDL_FreeSurface(item.second);
    }

    resource_map->clear();
}

#endif // HPP_RESOURCE