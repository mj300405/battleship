#include"AI.h"

AI::AI() {
    std::random_device rd;
    rng = std::mt19937(rd());
    enemy = nullptr;
    board = nullptr;
    ships_placed = false;
    repeat_move = false;
    for (int row = 0; row < 10; ++row)
        for (int col = 0; col < 10; ++col)
            unshot_cells.insert({ row, col });
}

AI::AI(Player* enemy_player, Board* ai_board) {
    std::random_device rd;
    rng = std::mt19937(rd());
    enemy = enemy_player;
    board = ai_board;
    ships_placed = false;
    repeat_move = false;
    for (int row = 0; row < 10; ++row)
        for (int col = 0; col < 10; ++col)
            unshot_cells.insert({ row, col });
}

//std::pair<int, int> AI::make_move() {
//    // If there are no more unshot cells, return a default invalid move.
//    if (unshot_cells.empty()) {
//        return std::make_pair(-1, -1);
//    }
//
//    // Get a random index into unshot_cells.
//    int index = rng() % unshot_cells.size();
//
//    // Iterate to the randomly selected position.
//    auto iter = unshot_cells.begin();
//    std::advance(iter, index);
//
//    // Record the shot cell.
//    std::pair<int, int> shot_cell = *iter;
//
//    // Remove this cell from the set of unshot cells.
//    unshot_cells.erase(iter);
//
//    return shot_cell;
//}

std::pair<int, int> AI::make_move() {
    // Choose a random cell from the set.
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 99);
    auto it = std::next(unshot_cells.begin(), dist(rng) % unshot_cells.size());
    std::pair<int, int> chosen_cell = *it;

    // Remove the chosen cell from the set.
    unshot_cells.erase(it);
    std::cout << "AI making a move" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return chosen_cell;
}

void AI::place_ships() {
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 9);

    for (Ship& ship : board->get_ships()) {
        int row, col;
        bool placed = false;
        const int max_random_attempts = 100;

        // Try to randomly place the ship
        for (int attempt = 0; attempt < max_random_attempts; ++attempt) {
            row = dist(rng);
            col = dist(rng);
            ship.is_horizontal = dist(rng) % 2;

            if (board->place_ship(ship, row, col)) {
                placed = true;
                break;
            }
        }

        // If random placement failed, try every possible placement
        if (!placed) {
            for (row = 0; row < board->get_grid_height(); ++row) {
                for (col = 0; col < board->get_grid_width(); ++col) {
                    ship.is_horizontal = true;
                    if (board->place_ship(ship, row, col)) {
                        placed = true;
                        break;
                    }
                    ship.is_horizontal = false;
                    if (board->place_ship(ship, row, col)) {
                        placed = true;
                        break;
                    }
                }
                if (placed) break;
            }
        }

        // If ship couldn't be placed even after trying all possibilities, there's a problem
        if (!placed) {
            throw std::runtime_error("Failed to place all ships");
        }
    }
    std::cout<< "AI::place_ships() - exiting the function;" << std::endl;
}

Board* AI::get_board() {
    return board;
}


