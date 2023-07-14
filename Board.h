#pragma once
#include "SDL.h"
#include <iostream>
#include <vector>
#include "Ship.h"
#include "Connection.h"



class Board
{
private:
    int grid_cell_size;
    int grid_width;
    int grid_height;
    int grid_height_px;
    int grid_width_px;
    int grid[10][10];
    int grid_offset_x;
    int grid_offset_y;
    SDL_Color grid_background;
    SDL_Color grid_line_color;
    SDL_Color grid_cursor_ghost_color;
    SDL_Color grid_cursor_color;
    bool is_current_player;
    std::vector<std::vector<bool>> made_shots;
    std::pair<int, int> last_shot{ -1, -1 };
    bool last_shot_was_hit = false;

public:

    std::vector<Ship> ships;
    Ship* selected_ship;

    Board(int x, int y, bool is_current);

    ~Board();

    std::pair<int, int> get_last_shot() { return last_shot; };

    void draw(SDL_Renderer* renderer);

    void update_cursor(SDL_Rect* cursor_ghost, SDL_Event* event);

    void draw_cursor(SDL_Renderer* renderer, SDL_Rect* cursor_ghost);

    bool shoot(SDL_Event* event);

    bool shoot(int row, int col);

    void render_enemy_grid(SDL_Renderer* renderer);

    //bool place_ship(SDL_Event* event, Ship& ship, const std::string& game_id, Connection* conn = nullptr, bool multiplayer_mode = false);

    bool place_ship(SDL_Event* event, Ship& ship);

    //bool place_ship(Ship& ship, int row, int col, const std::string& game_id, Connection* conn = nullptr, bool multiplayer_mode = false);

    bool place_ship(Ship& ship, int row, int col);

    bool place_ship(Ship& ship);

    bool remove_ship(SDL_Event* event);

    void draw_ship(SDL_Renderer* renderer, Ship& ship);

    void rotate_ship(SDL_Event* event, Ship& ship);

    bool check_ship_placement(const Ship& ship);

    int get_grid_cell_size();

    int get_grid_offset_x();

    int get_grid_offset_y();

    Ship* get_ship_at(int row, int col);

    bool is_valid_ship_position(Ship& ship);

    bool is_game_over();

    void set_current_player(bool is_current);
    
    void draw_ships(SDL_Renderer* renderer);

    bool all_ships_sunk();

    int get_grid_height();

    int get_grid_width();

    std::vector<Ship>& get_ships();

    bool all_ships_placed();

    bool is_cell_empty(int row, int col);

    void reset();

    bool all_cells_shot();

    void set_ai_shot(int row, int col);

    void draw_x(SDL_Renderer* renderer, int row, int col, int r, int g, int b);

    void draw_own_board(SDL_Renderer* renderer);

    void draw_enemy_board(SDL_Renderer* renderer);

    bool was_last_shot_hit();

    //void on_succesful_placement(const Ship& ship, Connection* conn, bool multiplayer_mode, const std::string& game_id);

    nlohmann::json to_json();
};

