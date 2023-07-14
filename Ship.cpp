#include "Ship.h"
#include <ranges>

// Updated constructor
Ship::Ship(int size, int start_row, int start_col, bool is_horizontal)
    : size(size), start_row(start_row), start_col(start_col), is_horizontal(is_horizontal), is_placed(false), hits(size, false) {}

bool operator==(const Ship& a, const Ship& b) {
    return a.size == b.size &&
        a.start_row == b.start_row &&
        a.start_col == b.start_col &&
        a.is_horizontal == b.is_horizontal;
}



bool Ship::is_sunk() {
    return std::ranges::all_of(hits, [](bool hit) { return hit; });
}

void Ship::hit(int part_index) {
    if (part_index >= 0 && part_index < size) {
        hits[part_index] = true; // Mark the part as hit
    }
}

nlohmann::json Ship::to_json() {
	nlohmann::json j;
	j["size"] = size;
	j["start_row"] = start_row;
	j["start_col"] = start_col;
	j["is_horizontal"] = is_horizontal;
	j["is_placed"] = is_placed;
	return j;
}

void Ship::from_json(const nlohmann::json& j) {
    size = j["size"];
    start_row = j["start_row"];
    start_col = j["start_col"];
    is_horizontal = j["is_horizontal"];
    is_placed = j["is_placed"];
    hits = std::vector<bool>(size, false);
}
