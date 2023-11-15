#include "Button.h"
#include <iostream>

Button::Button(SDL_Rect rect, SDL_Color color, SDL_Texture* label, std::function<void()> callback)
    : rect(rect), color(color), label(label), callback(callback) {}

void Button::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderCopy(renderer, label, nullptr, &rect);
}

bool Button::contains_point(int x, int y) {
    return x >= rect.x && y >= rect.y && x <= rect.x + rect.w && y <= rect.y + rect.h;
}

void Button::click() {
    callback();
}

void Button::handle_event(SDL_Event* e) { std::cout << "ta co sie nie ma wywolac" << std::endl; }

TextField::TextField(SDL_Rect rect, SDL_Color color, SDL_Texture* label, std::string text, TTF_Font* font, SDL_Color textColor) : Button(rect, color, label, []() {}), text(text), font(font), textColor(textColor) {}


void TextField::render(SDL_Renderer* renderer) {
    Button::render(renderer);
    //std::cout<<"renderuje"<<std::endl;
    SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (!text_surface) {
        // Add error handling
        return;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (!text_texture) {
        // Add error handling
        SDL_FreeSurface(text_surface);
        return;
    }

    SDL_Rect dst = { rect.x, rect.y, text_surface->w, text_surface->h };
    SDL_RenderCopy(renderer, text_texture, NULL, &dst);
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

void TextField::set_text(const std::string text) {
    this->text = text;
}

std::string TextField::get_text() {
    return text;
}

void TextField::handle_event(SDL_Event* e) {
    //std::cout<<"to co sie ma wykonac"<<std::endl;
    if (e->type == SDL_TEXTINPUT || (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_BACKSPACE)) {
        if (e->type == SDL_TEXTINPUT)
            text += e->text.text;
        else if (!text.empty())
            text.pop_back();
    }
}