#if !defined(HPP_KEYBOARD)
#define HPP_KEYBOARD

#include <string>
using std::string;
using std::to_string;

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#include "../common/legend.hpp"
#include "../common/display.hpp"
#include "../common/keymap.hpp"
#include "../common/resource.hpp"

#include "inputfield.hpp"

#define KEY_WIDTH 62
#define KEY_HEIGHT 54

#define HOTKEY_KEYSET_ALT

#if !defined(HOTKEY_KEYSET_ALT)
#define HOTKEY_SPACE BUTTON_Y
#define HOTKEY_SPACE_RES RES_SMALL_Y
#define HOTKEY_BACKSPACE BUTTON_B
#define HOTKEY_BACKSPACE_RES RES_SMALL_B
#define HOTKEY_CANCEL BUTTON_X
#define HOTKEY_CANCEL_RES RES_BUTTON_X
#define HOTKEY_CANCEL_RES_ALT "X"
#else
#define HOTKEY_SPACE BUTTON_X
#define HOTKEY_SPACE_RES RES_SMALL_X
#define HOTKEY_BACKSPACE BUTTON_Y
#define HOTKEY_BACKSPACE_RES RES_SMALL_Y
#define HOTKEY_CANCEL BUTTON_B
#define HOTKEY_CANCEL_RES RES_BUTTON_B
#define HOTKEY_CANCEL_RES_ALT "B"
#endif

class Keyboard
{
private:
    Display* display;
    string initial_value = "";

    int current_layer = 0;
    int selected_x = 0;
    int selected_y = 0;
    int shift_mode = 0;
    int break_shift = 0;
    int last_keyup_time = SDL_GetTicks();
    char last_char = '\0';
    Uint8 keystate[320] = {};
    SDLKey key_repeat = SDLK_FIRST;
    SDLKey quit_key = SDLK_FIRST;
    int repeat_timer = 0;
    int kb_width = 10;
    int kb_height = 5;

    const char layers[3][4][10] = {
        {
            {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
            {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
            {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\b'},
            {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'}
        },
        {
            {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'},
            {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
            {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\b'},
            {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '-', '_', '?'}
        },
        {
            {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
            {'%', '\\', '|', '=', '[', ']', '<', '>', '{', '}'},
            {'@', '#', '$', '_', '&', '-', '+', '(', ')', '\b'},
            {'*', '"', '\'', ':', ';', '!', '?', '`', '~', '/'}
        }
    };
    const char controls[10] = {CHAR_SHIFT, CHAR_SPECIAL, CHAR_LEFT, CHAR_RIGHT, ' ', '\0', '\0', '\0', CHAR_OK, '\0'};

    ResourceMap res = loadResources(RES_BASE, {
        {RES_KEY_BACKSPACE},
        {RES_KEY_SHIFT "_0"},
        {RES_KEY_SHIFT "_1"},
        {RES_KEY_SHIFT "_2"},
        {RES_KEY_SPACE},
        {RES_KEY_CURSOR_LEFT},
        {RES_KEY_CURSOR_RIGHT},
        {RES_SMALL_L},
        {RES_SMALL_R},
        {RES_SMALL_L2},
        {RES_SMALL_R2},
        {RES_SMALL_START},
        {RES_BUTTON_A},
        {HOTKEY_BACKSPACE_RES},
        {HOTKEY_SPACE_RES},
        {HOTKEY_CANCEL_RES}
    });

    InputField input;
    Legend legend;

    void drawKey(char c, int selected, Rect keyrect);
    void drawLabel(string label, Rect* keyrect);
    void drawIcon(SDL_Surface* icon, string alt_label, Rect* keyrect);
    void drawHotkey(SDL_Surface* icon, Rect* keyrect);
    int spaceSelected(void);
    int okSelected(void);
    int pressKey(char c, int repeating = 0);
    char getSelectedChar(void);
    int isDoubleClick(char c);

public:
    bool cancelled = false;
    string title = "";

    Keyboard(Display* _display, string _value, string _title)
        : display(_display), initial_value(_value), title(_title)
    {
        input = InputField(display, _value);

        legend = Legend(display, new std::vector<LegendItem>{
            {res[RES_BUTTON_A], "Enter", "A"},
            {res[HOTKEY_CANCEL_RES], "Cancel", HOTKEY_CANCEL_RES_ALT}
        });

        render();
    };
    ~Keyboard()
    {
        freeResources(&res);
    };

    void render(void);
    string getValue(void) { return input.value; };
    bool handleKeyPress(SDLKey key, Uint8 type, int repeating);
};

int kbinput(Display* display, string title, string value, string* output)
{
    bool quit = false;
    Keyboard* kb = new Keyboard(display, value, title);

    auto input_handler = [&kb](SDLKey key, Uint8 type, int repeating) {
        return kb->handleKeyPress(key, type, repeating);
    };

    auto frame_handler = [display, input_handler](void) {
        return display->onInputEvent(input_handler);
    };
    
    while (!quit) {
        quit = display->requestFrame(frame_handler);
    }

    bool cancelled = kb->cancelled;

    if (!cancelled)
        value = kb->getValue();

    delete kb;

    *output = value;
    return cancelled;
}

void Keyboard::render()
{
    // Clear screen
    display->clear();

    // Header
    if (title.length() > 0)
        display->centerText(title, {320, 30});

    // Input field
    input.render();

    // Keyboard
    int z = current_layer;
    int keyboard_x = 150;

    for (int y = 0; y < 5; y++) {
        int pos_y = keyboard_x + y * KEY_HEIGHT;

        for (int x = 0; x < 10; x++) {
            int pos_x = 10 + x * KEY_WIDTH;

            if (y < 4)
                drawKey(layers[z][y][x], selected_x == x && selected_y == y, {pos_x, pos_y, KEY_WIDTH, KEY_HEIGHT});
            else if (controls[x] != '\0') {
                switch (controls[x])
                {
                case ' ':
                    drawKey(' ', spaceSelected(), {pos_x, pos_y, KEY_WIDTH * 4, KEY_HEIGHT});
                    break;
                case CHAR_OK:
                    drawKey(CHAR_OK, okSelected(), {pos_x, pos_y, KEY_WIDTH * 2, KEY_HEIGHT});
                    break;
                default:
                    drawKey(controls[x], selected_x == x && selected_y == y, {pos_x, pos_y, KEY_WIDTH, KEY_HEIGHT});
                    break;
                }
            }
        }
    }

    // Footer
    legend.render();

    display->flip();
}

void Keyboard::drawKey(char c, int selected, Rect keyrect)
{
    if (selected)
        display->fillRect(keyrect, display->COLOR_FRAME);

    display->fillRect(
        keyrect.addPadding(1),
        selected ? display->COLOR_SELECTED : display->COLOR_FACE
    );

    switch (c)
    {
    case '\b':
        drawIcon(res[RES_KEY_BACKSPACE], "BKSP", &keyrect);
        drawHotkey(res[HOTKEY_BACKSPACE_RES], &keyrect);
        break;
    case CHAR_SHIFT:
        if (current_layer == 2)
            break;
        drawIcon(res[RES_KEY_SHIFT "_" + to_string(shift_mode)], "SHFT", &keyrect);
        drawHotkey(res[RES_SMALL_L2], &keyrect);
        break;
    case CHAR_SPECIAL:
        drawLabel(current_layer == 2 ? (shift_mode == 0 ? "abc" : "ABC") : "?#", &keyrect);
        drawHotkey(res[RES_SMALL_R2], &keyrect);
        break;
    case CHAR_LEFT:
        drawIcon(res[RES_KEY_CURSOR_LEFT], "<", &keyrect);
        drawHotkey(res[RES_SMALL_L], &keyrect);
        break;
    case CHAR_RIGHT:
        drawIcon(res[RES_KEY_CURSOR_RIGHT], ">", &keyrect);
        drawHotkey(res[RES_SMALL_R], &keyrect);
        break;
    case ' ':
        drawIcon(res[RES_KEY_SPACE], "SPACE", &keyrect);
        drawHotkey(res[HOTKEY_SPACE_RES], &keyrect);
        break;
    case CHAR_OK:
        drawLabel("OK", &keyrect);
        drawHotkey(res[RES_SMALL_START], &keyrect);
        break;
    default:
        drawLabel({c}, &keyrect);
        break;
    }
}

void Keyboard::drawLabel(string label, Rect* keyrect)
{
    TTF_Font* font = label.length() == 1 ? display->fonts.label : display->fonts.regular;
    display->centerText(label, {keyrect->getCenterX(), keyrect->getCenterY()}, font, &display->COLOR_LABEL);
}

void Keyboard::drawIcon(SDL_Surface* icon, string alt_label, Rect* keyrect)
{
    if (!icon)
        return drawLabel(alt_label, keyrect);
    
    display->blit(icon, {
        keyrect->getCenterX() - icon->w / 2,
        keyrect->getCenterY() - icon->h / 2,
        icon->w,
        icon->h
    });
}

void Keyboard::drawHotkey(SDL_Surface* icon, Rect* keyrect)
{
    if (!icon)
        return;

    display->blit(icon, {
        keyrect->x + keyrect->w - icon->w,
        keyrect->y,
        icon->w,
        icon->h
    });
}

int Keyboard::spaceSelected(void)
{
    return selected_y == 4 && selected_x >= 4 && selected_x <= 7;
}

int Keyboard::okSelected(void)
{
    return selected_y == 4 && selected_x >= 8 && selected_x <= 9;
}

int Keyboard::pressKey(char c, int repeating)
{
    int changed = 0;

    switch (c)
    {
    // Shift
    case CHAR_SHIFT:
        if (repeating) break;
        if (current_layer == 2) break;
        if (isDoubleClick(c)) {
            shift_mode = 2;
            current_layer = 1;
        }
        else {
            shift_mode = current_layer = current_layer == 0 ? 1 : 0;
        }
        changed = 1;
        break;
    // Special characters
    case CHAR_SPECIAL:
        if (repeating) break;
        current_layer = current_layer == 2 ? (shift_mode == 0 ? 0 : 1) : 2;
        changed = 1;
        break;
    default:
        changed = input.inputChar(c, repeating);

        if (c < 32 || c > 126)
            break;

        if (keystate[BUTTON_L2])
            break_shift = 1;

        if (c != ' ' && shift_mode == 1 && !keystate[BUTTON_L2]) {
            shift_mode = 0;
            if (current_layer == 1)
                current_layer = 0;
        }

        break;
    }

    last_char = c;

    return changed;
}

char Keyboard::getSelectedChar(void)
{
    if (spaceSelected())
        return ' ';
    else if (okSelected())
        return CHAR_OK;
    else if (selected_y == 4)
        return controls[selected_x];
    return layers[current_layer][selected_y][selected_x];
}

int Keyboard::isDoubleClick(char c)
{
    int delta = SDL_GetTicks() - last_keyup_time;
    
    if (delta > 300)
        return 0;

    if (last_char != c)
        return 0;

    return 1;
}

bool Keyboard::handleKeyPress(SDLKey key, Uint8 type, int repeating)
{
    int changed = 0;

    if (type == SDL_KEYUP) {
        last_keyup_time = SDL_GetTicks();

        // Cancel if MENU or cancel is pressed
        if (key == BUTTON_MENU || key == HOTKEY_CANCEL) {
            input.value = initial_value;
            cancelled = true;
            return true;
        }

        if (key == BUTTON_L2 && break_shift) {
            break_shift = 0;
            shift_mode = 0;
            if (current_layer == 1)
                current_layer = 0;
            render();
        }

        return false;
    }

    char pressed_char = 0;

    switch (key)
    {
    case BUTTON_RIGHT:
        if (spaceSelected()) selected_x = 8;
        else if (okSelected() && !repeating) selected_x = 0;
        else if (selected_x < kb_width - 1) selected_x++;
        else if (repeating) return false;
        else selected_x = 0;
        changed = 1;
        break;
    case BUTTON_LEFT:
        if (spaceSelected()) selected_x = 3;
        else if (okSelected()) selected_x = 7;
        else if (selected_x > 0) selected_x--;
        else if (repeating) return false;
        else selected_x = kb_width - 1;
        changed = 1;
        break;
    case BUTTON_DOWN:
        if (selected_y < kb_height - 1) selected_y++;
        else if (repeating) return false;
        else selected_y = 0;
        changed = 1;
        break;
    case BUTTON_UP:
        if (selected_y > 0) selected_y--;
        else if (repeating) return false;
        else selected_y = kb_height - 1;
        changed = 1;
        break;
    case BUTTON_L1:         pressed_char = CHAR_LEFT; break;
    case BUTTON_R1:         pressed_char = CHAR_RIGHT; break;
    case BUTTON_L2:         pressed_char = CHAR_SHIFT; break;
    case BUTTON_R2:         pressed_char = CHAR_SPECIAL; break;
    case BUTTON_A:          pressed_char = getSelectedChar(); break;
    case HOTKEY_BACKSPACE:  pressed_char = '\b'; break;
    case HOTKEY_SPACE:      pressed_char = ' '; break;
    case BUTTON_START:      pressed_char = CHAR_OK; break;
    case BUTTON_SELECT:     input.selectAll(); changed = 1; break;
    default: break;
    }

    if (pressed_char == CHAR_OK)
        return true;

    if (pressed_char != 0)
        changed = pressKey(pressed_char, repeating);

    if (changed)
        render();

    return false;
}

#endif // HPP_KEYBOARD
