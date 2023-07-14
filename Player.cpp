#include "Board.h"
#include "Player.h"

Player::Player() : own_board(nullptr), is_turn(false), repeat_move(false) {};

Player::Player(Board* board) : own_board(board), is_turn(false) , repeat_move(false) {};

bool Player::get_is_turn() { return is_turn; }

void Player::set_is_turn(bool is_turn) { this->is_turn = is_turn; }

Board* Player::get_board() { return own_board; }

void Player::end_turn() { is_turn = false; }

void Player::start_turn() { is_turn = true; }

Ship* Player::get_selected_ship() {
    return own_board->selected_ship;
}

bool Player::get_repeat_move() { return repeat_move; }

void Player::set_repeat_move(bool val) { repeat_move = val; }