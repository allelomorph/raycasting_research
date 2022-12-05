#include "Layout.hh"
#include "Matrix.hh"   // Vector2d

#include <iostream>
#include <fstream>     // ifstream
#include <sstream>     // ostringstream
#include <string>


// parsing map file into map in memory
// Map 2d array representation will have row order inverted from map file order.
//   This allows consistency across the human-readable maze map file, indexed
//   access to the map array, and vector calculations in the raycasting engine:
//   - map file: north is up, +y is up
//   - map array with [i][j] ([y][x]): +i(+y) is up
//   - vector values: +y is up
Layout::Layout(std::string map_filename, Vector2d& pos) {
    std::ifstream map_file(map_filename);
    std::ostringstream err_msg;
    if (!map_file.is_open()) {
        err_msg << "Could not open map file: " << map_filename;
        throw std::runtime_error(err_msg.str());
    }
    std::string line;
    bool viable_start_exists = false;
    bool chosen_start_exists = false;
    rows = 0;
    cols = 0;
    std::vector<int> row;
    std::cout << "parsing map file:\n";
    // empty lines end map parsing
    while (std::getline(map_file, line) && line.size()) {
        ++rows;
        std::cout << line << '\n';
        for (uint32_t i {0}; line[i] != '\0'; ++i) {
            if (line[i] == 'x') {
                if (chosen_start_exists) {
                    err_msg <<
                        "Mulitple start points specified. Check your map!!";
                    throw std::runtime_error(err_msg.str());
                }
                chosen_start_exists = true;
                viable_start_exists = true;
                pos(0) = i;         // x
                pos(1) = rows - 1;  // y
                row.emplace_back(0);
            } else if (std::isdigit(line[i])) {
                // default start is first empty map tile found
                //   (most northern, then eastern)
                if (!viable_start_exists && line[i] == '0') {
                    viable_start_exists = true;
                    pos(0) = i;         // x
                    pos(1) = rows - 1;  // y
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
        cols = std::max(static_cast<uint16_t>(line.size()), cols);
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
        if (row.size() < cols)
            row.resize(cols, 1);
        // add outer E and W walls (even if already bounded)
        row.insert(row.begin(), 1);
        row.push_back(1);
    }
    cols += 2;
    // add outer S and N walls (even if already bounded)
    map.insert(map.begin(), std::vector<int>(cols, 1));
    map.push_back(std::vector<int>(cols, 1));
    rows += 2;

    // set starting position
    //   (+1.0 to each dim to account for offset of added outer boundary walls
    //     to N and W, and +0.5 to start in the center of designated grid square)
    pos(0) += 1.5;
    pos(1) += 1.5;

    std::cout << "parsed map:\n";
    for (auto r_it { map.rbegin() }; r_it != map.rend(); ++r_it) {
        for (const auto &tile : *r_it)
            std::cout << tile;
        std::cout << "\n";
    }
}
