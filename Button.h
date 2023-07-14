#pragma once

#include "SDL.h"
#include "SDL_ttf.h"
#include <functional>
#include <string>

class Button {
protected:
    SDL_Rect rect;
    SDL_Color color;
    SDL_Texture* label;
    std::function<void()> callback;

public:
    Button(SDL_Rect rect, SDL_Color color, SDL_Texture* label, std::function<void()> callback);

    virtual void render(SDL_Renderer* renderer);

    bool contains_point(int x, int y);

    void click();

    virtual void handle_event(SDL_Event* e);

    virtual void set_text(std::string text) {}

    virtual std::string get_text() { return ""; }

    virtual ~Button() {}
};

class TextField : public Button {
private:
    std::string text;
    SDL_Color textColor;
    TTF_Font* font;
public:
    TextField(SDL_Rect rect, SDL_Color color, SDL_Texture* label, std::string text, TTF_Font* font, SDL_Color textColor);

    void render(SDL_Renderer* renderer);

    void set_text(std::string text);

    std::string get_text();

    void handle_event(SDL_Event* e);

    void click() = delete;

    ~TextField() {}
};