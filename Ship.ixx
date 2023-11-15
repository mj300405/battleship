module;

#include <vector>
#include <ranges>
#include "nlohmann/json.hpp"

export module ship;

export class Ship {
public:
    int size;
    int start_row;
    int start_col;
    bool is_horizontal;
    bool is_placed;
    std::vector<bool> hits;

    Ship() = default;

    Ship(int size, int start_row, int start_col, bool is_horizontal)
        : size(size), start_row(start_row), start_col(start_col), is_horizontal(is_horizontal), is_placed(false), hits(size, false) {}

    friend bool operator==(const Ship& a, const Ship& b) {
        return a.size == b.size &&
            a.start_row == b.start_row &&
            a.start_col == b.start_col &&
            a.is_horizontal == b.is_horizontal;
    }

    bool is_sunk() {
        return std::ranges::all_of(hits.begin(), hits.end(), [](bool hit) { return hit; });
    }

    void hit(int part_index) {
        if (part_index >= 0 && part_index < size) {
            hits[part_index] = true; // Mark the part as hit
        }
    }

    nlohmann::json to_json() {
        nlohmann::json j;
        j["size"] = size;
        j["start_row"] = start_row;
        j["start_col"] = start_col;
        j["is_horizontal"] = is_horizontal;
        j["is_placed"] = is_placed;
        j["hits"] = hits;
        return j;
    }

    void from_json(const nlohmann::json& j) {
        size = j["size"];
        start_row = j["start_row"];
        start_col = j["start_col"];
        is_horizontal = j["is_horizontal"];
        is_placed = j["is_placed"];
        hits = j["hits"].get<std::vector<bool>>();
    }
};
