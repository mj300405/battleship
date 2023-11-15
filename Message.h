#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class GameMessage {
public:
    GameMessage(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color);
    ~GameMessage();

    void display(int x, int y);

    void update_message(const std::string& message);

private:
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect rect;
    TTF_Font* font;
    SDL_Color color;
};
