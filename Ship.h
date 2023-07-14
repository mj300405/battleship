#pragma once
#include <vector>
#include"nlohmann/json.hpp"

class Ship
{
public:
    int size;
    int start_row;
    int start_col;
    bool is_horizontal;
    bool is_placed;
    std::vector<bool> hits;  // updated from 'int hits;'

    Ship() = default;

    Ship(int size, int start_row, int start_col, bool is_horizontal);

    friend bool operator==(const Ship& a, const Ship& b);

    void hit();

    bool is_sunk();

    void hit(int part_index);

    nlohmann::json to_json();

    void from_json(const nlohmann::json& j);
};
