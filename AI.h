#pragma once

#include "Board.h"
#include "Player.h"
#include "Ship.h"
#include "random"
#include <set>



class AI {
private:
    Player* enemy;
    Board* board;
    std::mt19937 rng;
    bool ships_placed;
    std::set<std::pair<int, int>> unshot_cells;
    bool repeat_move;

public:
    AI();

    AI(Player* enemy_player, Board* ai_board);

    std::pair<int, int> make_move();

    void place_ships();

    Board* get_board();
};