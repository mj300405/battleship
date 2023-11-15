#pragma once

#include <vector>
#include "Button.h"

class Menu {
private:
    std::vector<Button*> buttons;
    bool visible;

public:
    Menu();

    void add_button(Button* button);

    void render(SDL_Renderer* renderer);

    void handle_click(int x, int y);

    void show();

    void hide();

    bool is_visible() const;

    std::vector<Button*> get_buttons();

    ~Menu();
};