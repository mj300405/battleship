#pragma once

#include "Board.h"
#include <string>

class Player {
private:
    Board* own_board; // A board object to represent the player's own board.
    bool is_turn; // Boolean to manage the turn of the player.
    bool repeat_move;

public:
    Player(Board* board);

    Player();

    // Getter and Setter for isTurn
    bool get_is_turn();

    void set_is_turn(bool is_turn);

    // Getter for ownBoard
    Board* get_board();

    void end_turn();

    void start_turn();

    Ship* get_selected_ship();

    bool get_repeat_move();

    void set_repeat_move(bool val);

};
