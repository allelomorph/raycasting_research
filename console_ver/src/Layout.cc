#include "Layout.hh"
#include "Matrix.hh"   // Vector2d

#include <iostream>
#include <fstream>     // ifstream
#include <sstream>     // ostringstream
#include <string>


// Map 2d array representation will have row order inverted from map file order.
//   This allows consistency across the human-readable maze map file, indexed
//   access to the map array, and vector calculations in the raycasting engine:
//   - map file: north is up, +y is up
//   - map array with [i][j] ([y][x]): +i(+y) is up
//   - vector values: +y is up
void Layout::loadMapFile(const std::string& map_filename, Vector2d& player_pos) {
    std::ifstream map_file(map_filename);
    std::ostringstream err_msg;
    if (!map_file.is_open()) {
        err_msg << "Could not open map file: " << map_filename;
        throw std::runtime_error(err_msg.str());
    }
    std::string line;
    bool viable_start_exists { false };
    bool chosen_start_exists { false };
    uint16_t row_ct { 0 };
    uint16_t col_ct { 0 };
    std::vector<tile_type> row;
    // empty lines end map parsing
    while (std::getline(map_file, line) && line.size()) {
        ++row_ct;
        for (uint32_t i {0}; line[i] != '\0'; ++i) {
            if (line[i] == 'x') {
                if (chosen_start_exists) {
                    err_msg <<
                        "Mulitple start points specified. Check your map!!";
                    throw std::runtime_error(err_msg.str());
                }
                chosen_start_exists = true;
                viable_start_exists = true;
                player_pos(0) = i;           // x
                player_pos(1) = row_ct - 1;  // y
                row.emplace_back(0);
            } else if (std::isdigit(line[i])) {
                // default start is first empty map tile found
                //   (most northern, then eastern)
                if (!viable_start_exists && line[i] == '0') {
                    viable_start_exists = true;
                    player_pos(0) = i;           // x
                    player_pos(1) = row_ct - 1;  // y
                }
                // TBD: C++ idiomatic conversion of digit char?
                row.emplace_back(line[i] - '0');
            } else if (line[i] == ' ') {  // \t width unpredictable
                // pad out non-rectangular western maze edges on map
                row.emplace_back(1);
            } else {
                err_msg <<
                    "Unrecognized character in map encoding. Check your map!!";
                throw std::runtime_error(err_msg.str());
            }
        }
        // columns set to max width of variable width maps
        col_ct = std::max(static_cast<uint16_t>(line.size()), col_ct);
        // insert at start of map array to invert row order from map file
        map.emplace(map.begin(), row);
        row.clear();
    }
    map_file.close();
    if (!viable_start_exists) {
        err_msg <<
            "No valid start location possible! Check your map!!";
        throw std::runtime_error(err_msg.str());
    }
    for (auto & row: map) {
        // pad out non-rectangular eastern maze edges on map
        if (row.size() < col_ct)
            row.resize(col_ct, 1);
        // add outer E and W walls (even if already bounded)
        row.insert(row.begin(), 1);
        row.push_back(1);
    }
    col_ct += 2;
    // add outer S and N walls (even if already bounded)
    map.insert(map.begin(), std::vector<tile_type>(col_ct, 1));
    map.push_back(std::vector<tile_type>(col_ct, 1));
    row_ct += 2;

    // +1.0 to each dim to account for offset of added outer boundary walls
    //   to N and W, and +0.5 to start in the center of designated grid square
    player_pos(0) += 1.5;
    player_pos(1) += 1.5;

    w = col_ct;
    h = row_ct;

    std::cout << "Parsed map file: " << map_filename << "\n";
}
