#include "Message.h"
#include <iostream>

GameMessage::GameMessage(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color)
    : renderer(renderer), font(font), color(color), texture(nullptr) {}

GameMessage::~GameMessage() {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}

void GameMessage::update_message(const std::string& message) {
    if (texture) {
        SDL_DestroyTexture(texture);
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, message.c_str(), color);
    if (!surface) {
        std::cerr << "Error in TTF_RenderText_Solid: " << TTF_GetError() << std::endl;
        return;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Error in SDL_CreateTextureFromSurface: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }

    rect = { 0, 0, surface->w, surface->h };

    SDL_FreeSurface(surface);
}

void GameMessage::display(int x, int y) {
    rect.x = x;
    rect.y = y;
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
}
