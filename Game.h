#pragma once

#include "SDL.h"
#include "SDL_ttf.h"
#include "Player.h"
#include "Board.h"
#include "AI.h"
#include "Message.h"
#include "Menu.h"
#include "Connection.h"
#include <mutex>
#include <thread>

enum class GameMode {
    SINGLE_PLAYER,
    MULTIPLAYER,
    EXIT
};

enum class GamePhase {
    MENU,
    SETUP,
    PLAY,
    WAITING,
    GAME_OVER
};

class Game {
private:
    Player player1, player2;
    Player* current_player;
    AI ai;
    SDL_Window* window;
    SDL_Renderer* renderer;
    int window_width;
    int window_height;
    SDL_Rect cursor_ghost1; // Cursor ghost for player1
    SDL_Rect cursor_ghost2; // Cursor ghost for player2

    SDL_Color grid_background;
    SDL_Rect grid_cursor_ghost;
    SDL_bool mouse_active;
    SDL_bool mouse_hover;
    bool game_running;
    GamePhase game_phase;
    GameMessage* gameMessage;
    TTF_Font* font;
    GameMode game_mode;
    Menu* gameMenu;
    bool is_multiplayer;
    Connection* conn;  // Add a Connection pointer
    std::string game_id;
    SDL_Event* event;
    Menu* game_over_menu;
    bool ultimate_end;
    Menu* multiplayer_menu;

    std::mutex bool_mutex;
    bool is_data_ready;
    nlohmann::json recived_data;
    bool is_joining;
public:

    Game(const std::string& title);

    ~Game();

    bool run_game_loop();

    void handle_phase_menu();

    void handle_phase_setup();

    void handle_phase_play();

    void render(); // Renders game state to the window

    void setup_game_menu();

    void exit_game();

    void start_multi_player_game();

    void start_single_player_game();

    void handle_incoming_message(const std::string& message);

    void setup_phase_mouse_click();

    void handle_phase_game_over();

    void setup_game_over_menu();

    void setup_multiplayer_menu();

    std::vector<Ship> json_to_ships_vector(const nlohmann::json& j);

    void handle_phase_waiting();
};