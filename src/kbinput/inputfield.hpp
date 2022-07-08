#if !defined(HPP_INPUTFIELD)
#define HPP_INPUTFIELD

#include <string>
#include "../common/display.hpp"
#include "../common/keymap.hpp"

class InputField
{
private:
    Display* display;
    int all_selected = 0;
    int cursor_pos = 0;

public:
    std::string value = "";

    InputField(Display* _display, std::string _value) : display(_display), value(_value), cursor_pos(_value.length())
    {
        if (cursor_pos > 0)
            all_selected = 1;
    };
    InputField(void) {};
    ~InputField() {};

    void render(void);
    int inputChar(char c, int repeating);
    void selectAll(void);
};

void InputField::render(void)
{
    SDL_Surface* text_surface;

    int pad = 10;
    Rect outer_rect = {10, 60, 620, 80};
    display->fillRect(&outer_rect, display->COLOR_FRAME);
    Rect inner_rect = {11, 61, 618, 78};
    display->fillRect(&inner_rect, display->COLOR_BG);

    int cursor_x = inner_rect.x + pad;

    if (value.length() > 0) {
        int before_x = 0;
        std::string before_cursor = value.substr(0, cursor_pos);
        
        if (cursor_pos > 0) {
            text_surface = TTF_RenderUTF8_Blended(display->fonts.display, before_cursor.c_str(), display->COLOR_FG);
            before_x = text_surface->w;
            cursor_x += before_x;
            SDL_FreeSurface(text_surface);
        }

        if (cursor_x > inner_rect.w)
            cursor_x = inner_rect.w;

        int overflow_w = inner_rect.w - 2 * pad;

        text_surface = TTF_RenderUTF8_Blended(display->fonts.display, value.c_str(), display->COLOR_FG);
        int text_x = before_x - overflow_w;

        if (text_x < 0)
            text_x = 0;
        
        int text_crop_w = text_surface->w;
        if (text_crop_w > inner_rect.w - pad) {
            text_crop_w = inner_rect.w;
            if (text_x == 0)
                text_crop_w -= pad;
        }

        Rect srcrect = {text_x > 0 ? text_x - pad : 0, 0, text_crop_w + pad, 50};

        text_x = text_x == 0 ? pad : 0;
        text_x += inner_rect.x;
        int text_y = inner_rect.getCenterY() - text_surface->h / 2;
        Rect dstrect = {text_x, text_y, text_crop_w < overflow_w ? text_crop_w : inner_rect.w - pad, 50};

        if (all_selected)
            display->fillRect(&dstrect, display->COLOR_SELECTED);

        display->blit(text_surface, &srcrect, &dstrect);
        SDL_FreeSurface(text_surface);
    }

    // Cursor
    if (!all_selected)
        display->fillRect({cursor_x - 1, inner_rect.y + inner_rect.h / 2 - 25, 2, 50}, display->COLOR_FG);
}

void InputField::selectAll(void)
{
    all_selected = 1;
    cursor_pos = value.length();
}

int InputField::inputChar(char c, int repeating)
{
    int changed = 0;

    switch (c)
    {
    // Backspace
    case '\b':
        if (all_selected) {
            all_selected = 0;
            cursor_pos = 0;
            value = "";
            changed = 1;
        }
        else if (cursor_pos > 0) {
            value.erase(--cursor_pos, 1);
            changed = 1;
        }
        break;
    // Cursor left
    case CHAR_LEFT:
        if (all_selected) {
            all_selected = 0;
            cursor_pos = 0;
            changed = 1;
        }
        else if (cursor_pos > 0) {
            cursor_pos--;
            changed = 1;
        }
        break;
    // Cursor right
    case CHAR_RIGHT:
        if (all_selected) {
            all_selected = 0;
            changed = 1;
        }
        else if (cursor_pos < value.length()) {
            cursor_pos++;
            changed = 1;
        }
        break;
    default:
        if (all_selected) {
            all_selected = 0;
            cursor_pos = 0;
            value = "";
        }

        value.insert(cursor_pos++, 1, c);

        changed = 1;
        break;
    }

    return changed;
}

#endif // HPP_INPUTFIELD