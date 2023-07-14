#include "Menu.h"

Menu::Menu() : visible(true) {}

void Menu::add_button(Button* button) {
    buttons.push_back(button);
}

void Menu::render(SDL_Renderer* renderer) {
    if (!visible) {
        return;
    }

    for (Button* button : buttons) {
        button->render(renderer);
    }
}

void Menu::handle_click(int x, int y) {
    if (!visible) {
        return;
    }

    for (Button* button : buttons) {
        if (button->contains_point(x, y)) {
            button->click();
            return;
        }
    }
}

void Menu::show() {
    visible = true;
}

void Menu::hide() {
    visible = false;
}

bool Menu::is_visible() const {
    return visible;
}

std::vector<Button*> Menu::get_buttons() {
	return buttons;
}

Menu::~Menu(){}