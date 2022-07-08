#if !defined(HPP_DISPLAY)
#define HPP_DISPLAY

#include <iostream>
#include <string>
#include <functional>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#define FRAMES_PER_SECOND 60
#define KEY_REPEAT_DELAY 300
#define KEY_REPEAT_SPEED 30

#define MAIN_FONT "/mnt/SDCARD/miyoo/app/BPreplayBold.otf"
#define LABEL_FONT "/mnt/SDCARD/miyoo/app/Helvetica-Neue-2.ttf"

#define CENTER_NONE 0
#define CENTER_BOTH 1
#define CENTER_H 2
#define CENTER_V 3

struct Rect
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    SDL_Rect to_sdl(void);
    Rect addPadding(int padding);
    int getCenterX(void);
    int getCenterY(void);
};

SDL_Rect Rect::to_sdl(void)
{
    return {(int16_t)(x), (int16_t)(y), (uint16_t)(w), (uint16_t)(h)};
}

Rect Rect::addPadding(int padding)
{
    return {x + padding, y + padding, w - 2 * padding, h - 2 * padding};
}

int Rect::getCenterX(void)
{
    return x + w / 2;
}

int Rect::getCenterY(void)
{
    return y + h / 2;
}

struct DisplayFonts { TTF_Font *display; TTF_Font *regular; TTF_Font *label; };

class Display
{
private:
    Uint32 last_ticks = SDL_GetTicks(),
           acc_ticks = 0;
    SDLKey quit_key = SDLK_FIRST;
    bool request_quit = false;
    int repeat_timer = 0;
    int repeat_ticks = 0;
public:
    SDL_Color COLOR_BG = {0, 0, 0};
    SDL_Color COLOR_FG = {255, 255, 255};
    SDL_Color COLOR_LABEL = {255, 255, 255};
    SDL_Color COLOR_FRAME = {255, 255, 255};
    SDL_Color COLOR_FACE = {26, 26, 26};
    SDL_Color COLOR_SELECTED = {51, 51, 51};

    float time_step = 1000.f / FRAMES_PER_SECOND;
    SDL_Surface* screen = NULL;
    DisplayFonts fonts;
    Uint8 keystate[320] = {};
    SDLKey key_repeat = SDLK_FIRST;

    Display()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
            exit(1);
        }

        if (!(screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE))) {
            std::cerr << "Could not set video mode: " << SDL_GetError() << std::endl;
            SDL_Quit();
            exit(1);
        }

        // Hide the cursor
        SDL_ShowCursor(SDL_DISABLE);

        // Enable Unicode translation
        SDL_EnableUNICODE(1);

        // Load font
        TTF_Init();
        fonts = {
            .display = TTF_OpenFont(MAIN_FONT, 40),
            .regular = TTF_OpenFont(MAIN_FONT, 24),
            .label = TTF_OpenFont(LABEL_FONT, 30)
        };

        // std::cerr << "DISPLAY UP" << std::endl;
    }

    ~Display()
    {
        TTF_CloseFont(fonts.display);
        TTF_CloseFont(fonts.regular);
        TTF_CloseFont(fonts.label);
        SDL_FreeSurface(screen);
        SDL_Quit();
        // std::cerr << "DISPLAY DOWN" << std::endl;
    }

    bool onInputEvent(std::function<bool(SDLKey, Uint8, int)> input_handler);
    bool requestFrame(std::function<bool(void)> update_handler);
    void fillRect(Rect rect, SDL_Color color);
    void fillRect(Rect* rect, SDL_Color color);
    void blit(SDL_Surface* src, Rect* srcrect, Rect* dstrect);
    void blit(SDL_Surface* src, Rect* dstrect);
    void blit(SDL_Surface* src, Rect dstrect);
    void blit(SDL_Surface* src, Rect* srcrect, Rect dstrect);
    void blit(SDL_Surface* src, Rect srcrect, Rect* dstrect);
    void blit(SDL_Surface* src, Rect srcrect, Rect dstrect);
    Rect text(std::string value, Rect* dstrect, TTF_Font* font = NULL, SDL_Color* color = NULL, int center = CENTER_NONE);
    Rect text(std::string value, Rect dstrect, TTF_Font* font = NULL, SDL_Color* color = NULL, int center = CENTER_NONE);
    Rect text(std::string value, Rect dstrect, TTF_Font* font, SDL_Color color, int center = CENTER_NONE);
    Rect centerText(std::string value, Rect* dstrect, TTF_Font* font = NULL, SDL_Color* color = NULL);
    Rect centerText(std::string value, Rect dstrect, TTF_Font* font = NULL, SDL_Color* color = NULL);
    Rect centerText(std::string value, Rect dstrect, TTF_Font* font, SDL_Color color);
    int textWidth(std::string value, TTF_Font* font = NULL);
    int textHeight(std::string value, TTF_Font* font = NULL);
    void clear(void);
    void flip(void);
};

bool Display::onInputEvent(std::function<bool(SDLKey, Uint8, int)> input_handler)
{
    bool quit = false;
    SDL_Event event;

    // Poll for events
    while (SDL_PollEvent(&event)) {
        SDLKey key = event.key.keysym.sym;

        switch (event.type) {
            // Keyboard event
            case SDL_KEYDOWN:
                if (keystate[key])
                    break;
                keystate[key] = 1;
                key_repeat = key;
                repeat_timer = SDL_GetTicks();
                if (request_quit) break;
                request_quit = input_handler(key, event.type, 0);
                if (request_quit && quit_key == SDLK_FIRST)
                    quit_key = key;
                break;
            case SDL_KEYUP:
                if (!keystate[key])
                    break;
                keystate[key] = 0;
                if (request_quit) break;
                request_quit = input_handler(key, event.type, 0);
                break;
            case SDL_QUIT:
                quit = true;
                break;
            default:
                break;
        }
    }

    int ticks = SDL_GetTicks();
    int delta = ticks - repeat_timer;

    if (keystate[key_repeat] && delta > KEY_REPEAT_DELAY && ticks - repeat_ticks > KEY_REPEAT_SPEED) {
        repeat_ticks = ticks;
        if (!request_quit)
            input_handler(key_repeat, SDL_KEYDOWN, 1);
    }

    if (request_quit && !keystate[quit_key])
        quit = true;

    if (quit) {
        quit_key = SDLK_FIRST;
        request_quit = false;
    }

    return quit;
}

bool Display::requestFrame(std::function<bool(void)> update_handler)
{
    bool quit = false;

    Uint32 ticks = SDL_GetTicks();
    acc_ticks += ticks - last_ticks;
    last_ticks = ticks;

    while (!quit && acc_ticks >= time_step) {
        quit = update_handler();
        acc_ticks -= time_step;
    }

    return quit;
}

void Display::fillRect(Rect rect, SDL_Color color)
{
    fillRect(&rect, color);
}

void Display::fillRect(Rect* _rect, SDL_Color color)
{
    SDL_Rect rect = _rect->to_sdl();
    Uint32 color_int = SDL_MapRGB(screen->format, color.r, color.g, color.b);
    SDL_FillRect(screen, &rect, color_int);
}

void Display::blit(SDL_Surface* src, Rect* _srcrect, Rect* _dstrect)
{
    SDL_Rect srcrect = _srcrect->to_sdl();
    SDL_Rect dstrect = _dstrect->to_sdl();
    SDL_BlitSurface(src, &srcrect, screen, &dstrect);
}
void Display::blit(SDL_Surface* src, Rect* _dstrect)
{
    SDL_Rect dstrect = _dstrect->to_sdl();
    SDL_BlitSurface(src, NULL, screen, &dstrect);
}
void Display::blit(SDL_Surface* src, Rect dstrect) { blit(src, &dstrect); }
void Display::blit(SDL_Surface* src, Rect* srcrect, Rect dstrect) { blit(src, srcrect, &dstrect); }
void Display::blit(SDL_Surface* src, Rect srcrect, Rect* dstrect) { blit(src, &srcrect, dstrect); }
void Display::blit(SDL_Surface* src, Rect srcrect, Rect dstrect) { blit(src, &srcrect, &dstrect); }

Rect Display::text(std::string value, Rect* dstrect, TTF_Font* font, SDL_Color* color, int center)
{
    SDL_Surface* text = TTF_RenderUTF8_Blended(
        !font ? fonts.regular : font,
        value.c_str(),
        !color ? COLOR_FG : *color
    );
    Rect srcrect = {dstrect->x, dstrect->y, text->w, text->h};

    int x = center == CENTER_BOTH || center == CENTER_H ? dstrect->x - text->w / 2 : dstrect->x;
    int y = center == CENTER_BOTH || center == CENTER_V ? dstrect->y - text->h / 2 : dstrect->y;

    blit(text, {x, y});
    SDL_FreeSurface(text);

    return srcrect;
}
Rect Display::text(std::string value, Rect dstrect, TTF_Font* font, SDL_Color* color, int center) { return text(value, &dstrect, font, color, center); }
Rect Display::text(std::string value, Rect dstrect, TTF_Font* font, SDL_Color color, int center) { return text(value, &dstrect, font, &color, center); }
Rect Display::centerText(std::string value, Rect* dstrect, TTF_Font* font, SDL_Color* color) { return text(value, dstrect, font, color, CENTER_BOTH); }
Rect Display::centerText(std::string value, Rect dstrect, TTF_Font* font, SDL_Color* color) { return text(value, &dstrect, font, color, CENTER_BOTH); }
Rect Display::centerText(std::string value, Rect dstrect, TTF_Font* font, SDL_Color color) { return text(value, &dstrect, font, &color, CENTER_BOTH); }

int Display::textWidth(std::string value, TTF_Font* font)
{
    SDL_Surface* text = TTF_RenderUTF8_Blended(!font ? fonts.regular : font, value.c_str(), COLOR_FG);
    int text_width = text->w;
    SDL_FreeSurface(text);
    return text_width;
}

int Display::textHeight(std::string value, TTF_Font* font)
{
    SDL_Surface* text = TTF_RenderUTF8_Blended(!font ? fonts.regular : font, value.c_str(), COLOR_FG);
    int text_height = text->h;
    SDL_FreeSurface(text);
    return text_height;
}

void Display::clear(void)
{
    SDL_FillRect(screen, NULL, 0);
}

void Display::flip(void)
{
    SDL_Flip(screen);
}

#endif // HPP_DISPLAY
