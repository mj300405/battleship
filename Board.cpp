#include "Board.h"
#include "Ship.h"
#include <vector>
#include <algorithm>
#include "nlohmann/json.hpp"

Board::Board(int x, int y, bool is_current)
{
    grid_cell_size = 20;
    grid_width = 10;
    grid_height = 10;
    grid_height_px = grid_height * grid_cell_size;
    grid_width_px = grid_width * grid_cell_size;
    grid_offset_x = x;
    grid_offset_y = y;
    grid_background = { 22, 22, 22, 255 }; // Barely Black
    grid_line_color = { 44, 44, 44, 255 }; // Dark grey
    grid_cursor_ghost_color = { 44, 44, 44, 255 };
    is_current_player = is_current;
    made_shots = std::vector<std::vector<bool>>(grid_height, std::vector<bool>(grid_width, false));
    ships = {
        Ship(5, 0, 0, true),
        Ship(4, 0, 0, true),
        Ship(3, 0, 0, true),
        Ship(3, 0, 0, true),
        Ship(2, 0, 0, true)
    };
    selected_ship = nullptr;

    // initialize grid with 0's
    for (int row = 0; row < grid_height; ++row)
    {
        for (int col = 0; col < grid_width; ++col) {
            grid[row][col] = 0;
        }
    }
}

Board::~Board() {}

int Board::get_grid_cell_size()
{
    return grid_cell_size;
}

int Board::get_grid_offset_x()
{
    return grid_offset_x;
}

int Board::get_grid_offset_y()
{
    return grid_offset_y;
}

void Board::draw(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g, grid_line_color.b, grid_line_color.a);
    for (int x = grid_offset_x; x < 1 + grid_offset_x + grid_width_px; x += grid_cell_size)
    {
        SDL_RenderDrawLine(renderer, x, grid_offset_y, x, grid_offset_y + grid_height_px);
    }

    for (int y = grid_offset_y; y < 1 + grid_offset_y + grid_height_px; y += grid_cell_size)
    {
        SDL_RenderDrawLine(renderer, grid_offset_x, y, grid_offset_x + grid_width_px, y);
    }
}

void Board::update_cursor(SDL_Rect* cursor_ghost, SDL_Event* event)
{
    if ((event->motion.x >= grid_offset_x && event->motion.x < (grid_offset_x + grid_width_px) && event->motion.y >= grid_offset_y && event->motion.y < (grid_offset_y + grid_height_px)))
    {
        cursor_ghost->x = (event->motion.x / grid_cell_size) * grid_cell_size;
        cursor_ghost->y = (event->motion.y / grid_cell_size) * grid_cell_size;
    }
}

void Board::draw_cursor(SDL_Renderer* renderer, SDL_Rect* cursor_ghost)
{
    SDL_SetRenderDrawColor(renderer, grid_cursor_ghost_color.r, grid_cursor_ghost_color.g, grid_cursor_ghost_color.b, grid_cursor_ghost_color.a);
    SDL_RenderFillRect(renderer, cursor_ghost);
}

bool Board::shoot(SDL_Event* event) {
    std::cout << "Entering shoot by player" << std::endl;

    if ((event->motion.x >= grid_offset_x && event->motion.x < (grid_offset_x + grid_width_px) &&
        event->motion.y >= grid_offset_y && event->motion.y < (grid_offset_y + grid_height_px))) {

        int row = (event->motion.y - grid_offset_y) / grid_cell_size;
        int col = (event->motion.x - grid_offset_x) / grid_cell_size;

        std::cout << "Row: " << row << ", Col: " << col << std::endl;

        // No need to shoot again on already shot cell
        if (grid[row][col] == 9 || grid[row][col] == 8) {
            std::cout << "Cell already shot" << std::endl;
            return false;
        }
        

        if (grid[row][col] == 0) { //missed shot
            grid[row][col] = 9;
            made_shots[row][col] = true;
            last_shot = { row, col };
            last_shot_was_hit = false;

            std::cout << "Missed shot" << std::endl;
            return true;
        }
        else { // hit shot
            grid[row][col] = 8;
            made_shots[row][col] = true;

            Ship* hit_ship = get_ship_at(row, col);
            if (hit_ship) {
                int part_index = hit_ship->is_horizontal ? (col - hit_ship->start_col) : (row - hit_ship->start_row);
                hit_ship->hit(part_index);  // Notify the ship of a hit

                if (hit_ship->is_sunk()) {
                    std::cout << "Ship sunk!" << std::endl;
                }
            }

            last_shot = { row, col };
            last_shot_was_hit = true;

            std::cout << "Hit shot" << std::endl;
            return true;
        }
    }
    else {
        std::cout << "Invalid shot" << std::endl;
        return false;
    }
}

bool Board::shoot(int row, int col) {
    std::cout << "Attempting to shoot at (" << row << ", " << col << ")\n";
    if (row >= 0 && row < grid_height && col >= 0 && col < grid_width) {
        if (grid[row][col] == 0) { //missed shot
            grid[row][col] = 9;
            made_shots[row][col] = true;
            last_shot = { row, col };
            last_shot_was_hit = false;
            std::cout << "Missed shot\n";
            return false;
        }
        else if (grid[row][col] == 9 || grid[row][col] == 8) { //already shot once
            std::cout << "Cell already shot\n";
            return false;
        }
        else { // hit shot
            Ship* hit_ship = get_ship_at(row, col);
            if (hit_ship) {
                int part_index = hit_ship->is_horizontal ? (col - hit_ship->start_col) : (row - hit_ship->start_row);
                hit_ship->hit(part_index);
                grid[row][col] = 8;
                made_shots[row][col] = true;

                if (hit_ship->is_sunk()) {
                    std::cout << "Ship sunk!" << std::endl;
                }
            }

            last_shot = { row, col };
            last_shot_was_hit = true;
            std::cout << "Successful hit\n";
            return true;
        }
    }
    std::cout << "Invalid coordinates\n";
    return false;
}

void Board::render_enemy_grid(SDL_Renderer* renderer)
{
    //std::cout<<"entering render enemy grid"<<std::endl;
    for (int row = 0; row < grid_height; ++row)
    {
        for (int col = 0; col < grid_width; ++col)
        {
            SDL_Rect cell;
            cell.x = col * grid_cell_size + grid_offset_x;
            cell.y = row * grid_cell_size + grid_offset_y;
            cell.w = grid_cell_size;
            cell.h = grid_cell_size;

            if (made_shots[row][col]) {
                //std::cout << "Shot made at [" << row << "][" << col << "], grid status: " << grid[row][col] << std::endl; // Debug output
                switch (grid[row][col]) {
                case 9: // missed shot
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for misses
                    break;
                case 8: // successful hit
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green for hits
                    break;
                }
            }
            else {
                SDL_SetRenderDrawColor(renderer, grid_background.r, grid_background.g, grid_background.b, grid_background.a); // Default color for cells without shots
            }

            SDL_RenderFillRect(renderer, &cell);
        }
    }
}

void Board::draw_ship(SDL_Renderer* renderer, Ship& ship)
{
    SDL_Color ship_color = { 0, 0, 255, 255 }; // Blue
    for (int i = 0; i < ship.size; ++i)
    {
        SDL_Rect cell;
        if (ship.is_horizontal)
        {
            cell.x = (ship.start_col + i) * grid_cell_size + grid_offset_x;
            cell.y = ship.start_row * grid_cell_size + grid_offset_y;
        }
        else
        {
            cell.x = ship.start_col * grid_cell_size + grid_offset_x;
            cell.y = (ship.start_row + i) * grid_cell_size + grid_offset_y;
        }

        cell.w = grid_cell_size;
        cell.h = grid_cell_size;
        SDL_SetRenderDrawColor(renderer, ship_color.r, ship_color.g, ship_color.b, ship_color.a);
        SDL_RenderFillRect(renderer, &cell);
    }
}

bool Board::check_ship_placement(const Ship& ship) {
    for (int i = -1; i <= ship.size; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int row = ship.is_horizontal ? ship.start_row + j : (ship.start_row + i);
            int col = ship.is_horizontal ? (ship.start_col + i) : ship.start_col + j;
            if (row >= 0 && row < grid_height && col >= 0 && col < grid_width && grid[row][col] != 0) {
                return false;  // Overlapping with another ship
            }
        }
    }
    return true;
}

//bool Board::place_ship(SDL_Event* event, Ship& ship, const std::string& game_id, Connection* conn, bool multiplayer_mode) {
//    if (!(event->button.x >= grid_offset_x && event->button.x < (grid_offset_x + grid_width_px) && event->button.y >= grid_offset_y && event->button.y < (grid_offset_y + grid_height_px))) {
//        ship.start_row = -1;
//        ship.start_col = -1;
//        return false;
//    }
//
//    int temp_start_row = (event->button.y - grid_offset_y) / grid_cell_size;
//    int temp_start_col = (event->button.x - grid_offset_x) / grid_cell_size;
//
//    if (ship.is_horizontal && temp_start_col + ship.size - 1 >= grid_width) {
//        ship.start_row = -1;
//        ship.start_col = -1;
//        return false;
//    }
//
//    if (!ship.is_horizontal && temp_start_row + ship.size - 1 >= grid_height) {
//        ship.start_row = -1;
//        ship.start_col = -1;
//        return false;
//    }
//
//    ship.start_row = temp_start_row;
//    ship.start_col = temp_start_col;
//
//    if (!is_valid_ship_position(ship)) {
//        ship.start_row = -1;
//        ship.start_col = -1;
//        return false;
//    }
//
//    for (int i = 0; i < ship.size; ++i) {
//        if (ship.is_horizontal) {
//            grid[ship.start_row][ship.start_col + i] = 1;
//        }
//        else {
//            grid[ship.start_row + i][ship.start_col] = 1;
//        }
//    }
//
//    ship.is_placed = true;
//
//    on_succesful_placement(ship, conn, multiplayer_mode, game_id);
//
//    return true;
//}

bool Board::place_ship(SDL_Event* event, Ship& ship) {
    if (!(event->button.x >= grid_offset_x && event->button.x < (grid_offset_x + grid_width_px) && event->button.y >= grid_offset_y && event->button.y < (grid_offset_y + grid_height_px))) {
        ship.start_row = -1;
        ship.start_col = -1;
        return false;
    }

    int temp_start_row = (event->button.y - grid_offset_y) / grid_cell_size;
    int temp_start_col = (event->button.x - grid_offset_x) / grid_cell_size;

    if (ship.is_horizontal && temp_start_col + ship.size - 1 >= grid_width) {
        ship.start_row = -1;
        ship.start_col = -1;
        return false;
    }

    if (!ship.is_horizontal && temp_start_row + ship.size - 1 >= grid_height) {
        ship.start_row = -1;
        ship.start_col = -1;
        return false;
    }

    ship.start_row = temp_start_row;
    ship.start_col = temp_start_col;

    if (!is_valid_ship_position(ship)) {
        ship.start_row = -1;
        ship.start_col = -1;
        return false;
    }

    for (int i = 0; i < ship.size; ++i) {
        if (ship.is_horizontal) {
            grid[ship.start_row][ship.start_col + i] = 1;
        }
        else {
            grid[ship.start_row + i][ship.start_col] = 1;
        }
    }

    ship.is_placed = true;

    return true;
}

//bool Board::place_ship(Ship& ship, int row, int col, const std::string& game_id, Connection* conn, bool multiplayer_mode) {
//
//    if (ship.is_horizontal && col + ship.size - 1 >= grid_width) {
//        ship.start_row = -1;
//        ship.start_col = -1;
//        return false;
//    }
//
//    if (!ship.is_horizontal && row + ship.size - 1 >= grid_height) {
//        ship.start_row = -1;
//        ship.start_col = -1;
//        return false;
//    }
//
//    ship.start_row = row;
//    ship.start_col = col;
//
//    if (!is_valid_ship_position(ship)) {
//        ship.start_row = -1;
//        ship.start_col = -1;
//        return false;
//    }
//
//    for (int i = 0; i < ship.size; ++i) {
//        if (ship.is_horizontal) {
//            grid[ship.start_row][ship.start_col + i] = 1;
//        }
//        else {
//            grid[ship.start_row + i][ship.start_col] = 1;
//        }
//    }
//
//    ship.is_placed = true;
//
//    on_succesful_placement(ship, conn, multiplayer_mode, game_id);
//
//    return true;
//}

bool Board::place_ship(Ship& ship, int row, int col) {

    if (ship.is_horizontal && col + ship.size - 1 >= grid_width) {
        ship.start_row = -1;
        ship.start_col = -1;
        return false;
    }

    if (!ship.is_horizontal && row + ship.size - 1 >= grid_height) {
        ship.start_row = -1;
        ship.start_col = -1;
        return false;
    }

    ship.start_row = row;
    ship.start_col = col;

    if (!is_valid_ship_position(ship)) {
        ship.start_row = -1;
        ship.start_col = -1;
        return false;
    }

    for (int i = 0; i < ship.size; ++i) {
        if (ship.is_horizontal) {
            grid[ship.start_row][ship.start_col + i] = 1;
        }
        else {
            grid[ship.start_row + i][ship.start_col] = 1;
        }
    }

    ship.is_placed = true;

    return true;
}

bool Board::place_ship(Ship& ship) {
    for (int i = 0; i < ship.size; ++i) {
        if (ship.is_horizontal) {
            grid[ship.start_row][ship.start_col + i] = 1;
        }
        else {
            grid[ship.start_row + i][ship.start_col] = 1;
        }
    }

    ship.is_placed = true;

    return true;
}


bool Board::is_valid_ship_position(Ship& ship) {
    for (int i = 0; i < ship.size; ++i) {
        int ship_row = ship.start_row + (ship.is_horizontal ? 0 : i);
        int ship_col = ship.start_col + (ship.is_horizontal ? i : 0);

        // Check the cells on each side of the ship
        for (int j = -1; j <= 1; ++j) {
            for (int k = -1; k <= 1; ++k) {
                // Skip the center cell (ship cell)
                if (j == 0 && k == 0) {
                    continue;
                }

                int check_row = ship_row + j;
                int check_col = ship_col + k;

                // Exclude cells that are out of the grid
                if (check_row < 0 || check_row >= grid_height || check_col < 0 || check_col >= grid_width) {
                    continue;
                }

                // Check for overlapping with other ships
                for (const Ship& otherShip : ships) {
                    if (&otherShip != &ship && otherShip.is_placed) {  // Exclude the ship being placed
                        for (int m = 0; m < otherShip.size; ++m) {
                            int other_ship_row = otherShip.start_row + (otherShip.is_horizontal ? 0 : m);
                            int other_ship_col = otherShip.start_col + (otherShip.is_horizontal ? m : 0);
                            if (check_row == other_ship_row && check_col == other_ship_col) {
                                return false;  // Overlapping with another ship
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool Board::remove_ship(SDL_Event* event) {
    int row = (event->button.y - grid_offset_y) / grid_cell_size;
    int col = (event->button.x - grid_offset_x) / grid_cell_size;

    if (row < 0 || row >= grid_height || col < 0 || col >= grid_width || grid[row][col] != 1) {
        return false;  // No ship to remove
    }

    // Find the ship to remove
    Ship* ship_to_remove = nullptr;
    for (Ship& ship : ships) {
        if (ship.is_placed) {
            for (int i = 0; i < ship.size; ++i) {
                int ship_row = ship.is_horizontal ? ship.start_row : (ship.start_row + i);
                int ship_col = ship.is_horizontal ? (ship.start_col + i) : ship.start_col;

                if (ship_row == row && ship_col == col) {
                    ship_to_remove = &ship;
                    break;
                }
            }
        }

        if (ship_to_remove) {
            break; // Ship found, no need to check other ships
        }
    }

    if (ship_to_remove) {
        // Remove the ship from the grid
        for (int i = 0; i < ship_to_remove->size; ++i) {
            int row = ship_to_remove->is_horizontal ? ship_to_remove->start_row : (ship_to_remove->start_row + i);
            int col = ship_to_remove->is_horizontal ? (ship_to_remove->start_col + i) : ship_to_remove->start_col;
            grid[row][col] = 0;
        }

        ship_to_remove->is_placed = false;
        ship_to_remove->start_row = -1; // Invalidate the position
        ship_to_remove->start_col = -1; // Invalidate the position

        return true;
    }

    return false;
}

void Board::rotate_ship(SDL_Event* event, Ship& ship) {
    std::cout << "Rotating..." << std::endl;
    if (ship.is_placed) {
        int row = (event->button.y - grid_offset_y) / grid_cell_size;
        int col = (event->button.x - grid_offset_x) / grid_cell_size;

        if (row >= ship.start_row && row < ship.start_row + (ship.is_horizontal ? ship.size : 1) &&
            col >= ship.start_col && col < ship.start_col + (ship.is_horizontal ? 1 : ship.size)) {

            // Store the ship's original position and orientation
            int original_row = ship.start_row;
            int original_col = ship.start_col;
            bool original_orientation = ship.is_horizontal;

            // Try to rotate the ship
            ship.is_horizontal = !ship.is_horizontal;

            // If the ship is now horizontal, adjust the starting column
            if (ship.is_horizontal) {
                ship.start_col = std::min(ship.start_col, grid_width - ship.size);
            }
            // If the ship is now vertical, adjust the starting row
            else {
                ship.start_row = std::min(ship.start_row, grid_height - ship.size);
            }

            // Check if the ship's new position is valid
            if (!is_valid_ship_position(ship)) {
                // If the ship's new position is not valid, revert to the original position and orientation
                ship.start_row = original_row;
                ship.start_col = original_col;
                ship.is_horizontal = original_orientation;
            }
        }
    }
}

Ship* Board::get_ship_at(int row, int col) {
    for (Ship& ship : ships) {
        if (ship.is_placed && row >= ship.start_row && row < ship.start_row + (ship.is_horizontal ? 1 : ship.size) &&
            col >= ship.start_col && col < ship.start_col + (ship.is_horizontal ? ship.size : 1)) {
            return &ship;
        }
    }
    return nullptr;
}

bool Board::is_game_over()
{
    // Iterate over all ships in the vector
    for (auto& ship : ships)
    {
        // If any ship is not sunk, return false
        if (!ship.is_sunk())
        {
            return false;
        }
    }
    // If all ships are sunk, return true
    return true;
}

void Board::set_current_player(bool is_current) {
    is_current_player = is_current;
}

void Board::draw_ships(SDL_Renderer* renderer) {
    // Draw each cell
    for (int row = 0; row < grid_height; ++row) {
        for (int col = 0; col < grid_width; ++col) {
            SDL_Rect cell;
            cell.x = col * grid_cell_size + grid_offset_x;
            cell.y = row * grid_cell_size + grid_offset_y;
            cell.w = grid_cell_size;
            cell.h = grid_cell_size;

            if (get_ship_at(row, col)) {
                // If there's a ship here, draw the ship
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Cyan color for ship
                SDL_RenderFillRect(renderer, &cell);
            }

            if (made_shots[row][col]) {
                // If there was a shot here
                if (get_ship_at(row, col)) {
                    // If there's a ship here, it's a hit
                    // Draw a red 'X' for the hit cell
                    draw_x(renderer, row, col, 255, 0, 0);  // Red 'X' for hit
                }
                else {
                    // If there's no ship here, it's a miss
                    // Draw a white 'X' for the missed shot
                    draw_x(renderer, row, col, 255, 255, 255);  // White 'X' for miss
                }
            }
        }
    }
}

bool Board::all_ships_sunk() {
    for (Ship& ship : ships) {
        if (!ship.is_sunk()) {
            return false;
        }
    }
    return true;
}

int Board::get_grid_height() {
    return grid_height;
}

int Board::get_grid_width() {
	return grid_width;
}

std::vector<Ship>& Board::get_ships() {
	return ships;
}

bool Board::all_ships_placed() {
    for (Ship& ship : ships) {
        if (!ship.is_placed) {
            //std::cout << "Ship of size " << ship.size << " not placed.\n";
            return false;
        }
    }
    //1std::cout<< "All ships placed.\n";
    return true;
}

bool Board::is_cell_empty(int row, int col) {
    if (row >= 0 && row < grid_height && col >= 0 && col < grid_width) {
        return grid[row][col] == 0;
    }
    else {
        return false;
    }
}

void Board::reset() {
    // Reset all cells in the grid to empty
    for (int row = 0; row < grid_height; row++) {
        for (int col = 0; col < grid_width; col++) {
            grid[row][col] = 0;
        }
    }

    // Reset all ships so they're not placed
    for (Ship& ship : ships) {
        ship.is_placed = false;
        ship.start_row = -1;
        ship.start_col = -1;
        ship.is_horizontal = true;
    }
}

bool Board::all_cells_shot() {
    for (int row = 0; row < grid_height; ++row) {
        for (int col = 0; col < grid_width; ++col) {
            if (is_cell_empty(row, col)) {
                return false;
            }
        }
    }
    return true;
}

void Board::set_ai_shot(int row, int col) {
    made_shots[row][col] = true;
}

void Board::draw_x(SDL_Renderer* renderer, int row, int col, int r, int g, int b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255); // Set the color

    // Calculate the pixel coordinates of the corners of the cell
    int x1 = col * grid_cell_size + grid_offset_x;
    int y1 = row * grid_cell_size + grid_offset_y;
    int x2 = (col + 1) * grid_cell_size + grid_offset_x;
    int y2 = (row + 1) * grid_cell_size + grid_offset_y;

    // Define the thickness
    int thickness = 3; // Change this value to increase or decrease the thickness

    SDL_Rect line1, line2;

    // Calculate the dimensions of the lines
    line1.x = x1; line1.y = y1;
    line1.w = x2 - x1; line1.h = thickness;
    line2 = line1;
    line2.y = y2 - thickness;

    // Render the lines
    SDL_RenderFillRect(renderer, &line1);
    SDL_RenderFillRect(renderer, &line2);

    line1.x = x1; line1.y = y1;
    line1.w = thickness; line1.h = y2 - y1;
    line2 = line1;
    line2.x = x2 - thickness;

    // Render the lines
    SDL_RenderFillRect(renderer, &line1);
    SDL_RenderFillRect(renderer, &line2);
}

void Board::draw_own_board(SDL_Renderer* renderer) {
    // This function is similar to the previous draw method but also draws ships
    SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g, grid_line_color.b, grid_line_color.a);
    for (int x = grid_offset_x; x < 1 + grid_offset_x + grid_width_px; x += grid_cell_size)
    {
        SDL_RenderDrawLine(renderer, x, grid_offset_y, x, grid_offset_y + grid_height_px);
    }

    for (int y = grid_offset_y; y < 1 + grid_offset_y + grid_height_px; y += grid_cell_size)
    {
        SDL_RenderDrawLine(renderer, grid_offset_x, y, grid_offset_x + grid_width_px, y);
    }

    for (int row = 0; row < grid_height; ++row)
    {
        for (int col = 0; col < grid_width; ++col)
        {
            SDL_Rect cell;
            cell.x = col * grid_cell_size + grid_offset_x;
            cell.y = row * grid_cell_size + grid_offset_y;
            cell.w = grid_cell_size;
            cell.h = grid_cell_size;

            if (grid[row][col] == 0 && made_shots[row][col]) {
                // Missed shot, render with a certain color
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
                SDL_RenderFillRect(renderer, &cell);
            }
            else if (grid[row][col] > 0 && made_shots[row][col]) {
                // Successful hit, render with a different color
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
                SDL_RenderFillRect(renderer, &cell);
                // Draw an 'X' for the hit cell
                //draw_x(renderer, col * grid_cell_size + grid_offset_x, row * grid_cell_size + grid_offset_y);
            }
            else if (grid[row][col] > 0) {
                // Ship cell
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Cyan
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }
}

void Board::draw_enemy_board(SDL_Renderer* renderer) {
    // This function only draws cells and marks for the enemy's board
    SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g, grid_line_color.b, grid_line_color.a);
    for (int x = grid_offset_x; x < 1 + grid_offset_x + grid_width_px; x += grid_cell_size)
    {
        SDL_RenderDrawLine(renderer, x, grid_offset_y, x, grid_offset_y + grid_height_px);
    }

    for (int y = grid_offset_y; y < 1 + grid_offset_y + grid_height_px; y += grid_cell_size)
    {
        SDL_RenderDrawLine(renderer, grid_offset_x, y, grid_offset_x + grid_width_px, y);
    }

    for (int row = 0; row < grid_height; ++row)
    {
        for (int col = 0; col < grid_width; ++col)
        {
            SDL_Rect cell;
            cell.x = col * grid_cell_size + grid_offset_x;
            cell.y = row * grid_cell_size + grid_offset_y;
            cell.w = grid_cell_size;
            cell.h = grid_cell_size;

            // If the current player has shot here, render differently
            if (made_shots[row][col]) {
                if (grid[row][col] == 0) {
                    // Missed shot, render with a certain color
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
                }
                else {
                    // Hit shot, render with a different color
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
                    // Draw an 'X' for the hit cell
                    //draw_x(renderer, col * grid_cell_size + grid_offset_x, row * grid_cell_size + grid_offset_y);
                }
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }
}

bool Board::was_last_shot_hit() {
    return last_shot_was_hit;
}

//void Board::on_succesful_placement(const Ship& ship, Connection* conn, bool multiplayer_mode, const std::string& game_id) {
//    // If in multiplayer mode, send a message to the server.
//    if (multiplayer_mode) {
//        // Convert the ship's information to a JSON string or other message format.
//        nlohmann::json message;
//        message["game_id"] = game_id;  // Assuming you have a way to get the game_id
//        message["game_state"] = {
//            {"event", "ship_placed"},
//            {"ship_data", {
//                {"start_row", ship.start_row},
//                {"start_col", ship.start_col},
//                {"size", ship.size},
//                {"is_horizontal", ship.is_horizontal}
//            }}
//        };
//
//        // Convert to a string
//        std::string message_str = message.dump();
//
//        // Send the message
//        if (conn) {
//            conn->send_message(message_str);
//        }
//    }
//    else {
//        // Print a message in single player mode
//        std::cout << "Ship placed at "
//            << ship.start_row << ", "
//            << ship.start_col
//            << " with size " << ship.size
//            << (ship.is_horizontal ? " horizontally" : " vertically")
//            << std::endl;
//    }
//}
//
nlohmann::json Board::to_json() {
    nlohmann::json j;

    // Serialize ships vector
    for (int i = 0; i < ships.size(); i++) {
        j["ships"][i] = ships[i].to_json();  
    }

    return j;
}




//bool Board::shoot(SDL_Event* event)
//{
//    std::cout << "entering shoot by player" << std::endl;
//    if ((event->motion.x >= grid_offset_x && event->motion.x < (grid_offset_x + grid_width_px) && event->motion.y >= grid_offset_y && event->motion.y < (grid_offset_y + grid_height_px)))
//    {
//        int row = (event->motion.y - grid_offset_y) / grid_cell_size;
//        int col = (event->motion.x - grid_offset_x) / grid_cell_size;
//
//        // No need to shoot again on already shot cell
//        if (grid[row][col] == 9 || grid[row][col] == 8)
//        {
//            return false;
//        }
//
//        if (grid[row][col] == 0) //missed shot
//        {
//            grid[row][col] = 9;
//            made_shots[row][col] = true;
//            last_shot = { row, col };
//            last_shot_was_hit = false;
//            return true;
//        }
//        else { // hit shot
//            grid[row][col] = 8;
//            made_shots[row][col] = true;
//
//            Ship* hit_ship = get_ship_at(row, col);
//            if (hit_ship) {
//                int part_index = hit_ship->is_horizontal ? (col - hit_ship->start_col) : (row - hit_ship->start_row);
//                hit_ship->hit(part_index);  // Notify the ship of a hit
//
//                if (hit_ship->is_sunk()) {
//                    std::cout << "Ship sunk!" << std::endl;
//                }
//            }
//
//            last_shot = { row, col };
//            last_shot_was_hit = true;
//            return true;
//        }
//    }
//    return false;
//}

