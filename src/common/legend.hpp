#if !defined(HPP_LEGEND)
#define HPP_LEGEND

#include <string>
#include <vector>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#include "display.hpp"

#define START_X 640

#define ALIGN_LEFT 0
#define ALIGN_RIGHT 1

class LegendItem
{
public:
    SDL_Surface* icon;
    std::string label;
    std::string alt;
    LegendItem(SDL_Surface* _icon, std::string _label, std::string _alt) : icon(_icon), label(_label), alt(_alt) {};
    ~LegendItem(){};
};

class Legend
{
private:
    int legendText(int offset, std::string label, int align = ALIGN_RIGHT);
    int legendIcon(int offset, SDL_Surface* icon, std::string alt_label, int align = ALIGN_RIGHT);
    void renderItems(std::vector<LegendItem>* _items);
public:
    Display* display;
    std::vector<LegendItem>* items = NULL;
    std::vector<LegendItem>* extra_items = NULL;

    Legend(void) {};
    Legend(Display* _display, std::vector<LegendItem>* _items)
        : display(_display), items(_items) {};
    Legend(Display* _display, std::vector<LegendItem>* _items, std::vector<LegendItem>* _extra)
        : display(_display), items(_items), extra_items(_extra) {};
    ~Legend() {};

    void render(void);
};

void Legend::render(void)
{
    int offset = 0;

    if (items) {
        for (auto it = items->begin(); it != items->end(); ++it) {
            offset = legendText(offset + 20, it->label);
            offset = legendIcon(offset, it->icon, it->alt);
        }
    }

    if (extra_items) {
        offset = 0;
        for (auto it = extra_items->begin(); it != extra_items->end(); ++it) {
            offset = legendIcon(offset + 10, it->icon, it->alt, ALIGN_LEFT);
            offset = legendText(offset, it->label, ALIGN_LEFT);
        }
    }
}

int Legend::legendText(int offset, std::string label, int align)
{
    SDL_Surface* text = TTF_RenderUTF8_Blended(display->fonts.regular, label.c_str(), display->COLOR_FG);
    int text_width = text->w;
    int center = text_width >= 48 ? text_width / 2 : 24;
    int x = (align == ALIGN_RIGHT ? START_X - offset - text_width : offset) + center - text_width / 2;
    int y = 450 - text->h / 2;

    display->blit(text, {x, y});
    SDL_FreeSurface(text);

    offset += text_width;

    return offset;
}

int Legend::legendIcon(int offset, SDL_Surface* icon, std::string alt_label, int align)
{
    if (!icon) {
        legendText(offset + 10, alt_label);
        return offset + 48;
    }

    int x = align == ALIGN_RIGHT ? START_X - offset - icon->w : offset;
    int y = 450 - icon->h / 2;

    display->blit(icon, {x, y});

    offset += icon->w;

    return offset;
}

#endif // HPP_LEGEND
