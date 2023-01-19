#include "Layout.hh"
#include "Vector2d.hh"

#include <iostream>
#include <algorithm>   // max
#include <fstream>     // ifstream
#include <sstream>     // ostringstream
#include <string>


void Layout::loadMapFile(const std::string& map_filename, Vector2d& player_pos) {
    std::ifstream map_ifs(map_filename);
    std::ostringstream err_msg;
    if (!map_ifs.is_open()) {
        err_msg << "Could not open map file: " << map_filename;
        throw std::runtime_error(err_msg.str());
    }

    // get raw map grid size
    uint16_t row_ct { 0 };
    uint16_t col_ct { 0 };
    std::string line;
    while (std::getline(map_ifs, line) && line.size()) { // empty lines end map parsing
        ++row_ct;
        // columns set to max width of variable width maps
        col_ct = std::max(static_cast<uint16_t>(line.size()), col_ct);
    }
    // prevent unbounded stage by adding tiles for outer perimiter wall
    col_ct += 2;
    row_ct += 2;
    resize(col_ct, row_ct);

    // add N and S perimeter walls
    for (uint16_t col_i { 0 }; col_i < col_ct; ++col_i) {
        tile(col_i, 0) = 1;
        tile(col_i, row_ct - 1) = 1;
    }

    bool viable_start_exists { false };
    bool chosen_start_exists { false };
    map_ifs.clear();  // reset eof
    map_ifs.seekg(0);
    // y values inverted to convert map file row indices +y to coordinate grid +y
    for (int16_t row_i ( row_ct - 1 ); row_i >= 0; --row_i) {
        // add N and S perimeter walls
        if (row_i == 0 || row_i == row_ct - 1) {
            for (uint16_t col_i { 0 }; col_i < col_ct; ++col_i)
                tile(col_i, row_i) = 1;
            continue;
        }
        // add W perimeter wall
        tile(0, row_i) = 1;
        uint16_t col_i { 1 };
        std::getline(map_ifs, line);  // proven to succeed in sizing loop
        // pad out non-rectangular western maze edges on map
        for (; line[col_i - 1] == ' '; ++col_i)  // width of other whitespace unpredictable
            tile(col_i, row_i) = 1;
        // parse map line
        for (; line[col_i - 1]; ++col_i) {
            if (line[col_i - 1] == 'x') {
                if (chosen_start_exists) {
                    err_msg <<
                        "Mulitple start points specified. Check map file for errors.";
                    throw std::runtime_error(err_msg.str());
                }
                chosen_start_exists = true;
                viable_start_exists = true;
                player_pos.x = col_i;  // x
                player_pos.y = row_i;  // y
                tile(col_i, row_ct - 1 - row_i) = 0;
            } else if (std::isdigit(line[col_i - 1])) {
                // default start is first empty map tile found
                //   (most northern, then eastern)
                if (!viable_start_exists && line[col_i - 1] == '0') {
                    viable_start_exists = true;
                    player_pos.x = col_i;  // x
                    player_pos.y = row_i;  // y
                }
                // TBD: idiomatic conversion to integer?
                tile(col_i, row_i) = line[col_i - 1] - '0';
            } else {
                err_msg <<
                    "Unrecognized character in map encoding. Check map file for errors.";
                throw std::runtime_error(err_msg.str());
            }
        }
        // pad out non-rectangular eastern maze edges on map
        for (; col_i < col_ct - 1; ++col_i)
            tile(col_i, row_i) = 1;
        // add E perimeter wall
        tile(col_i, row_i) = 1;
    }
    map_ifs.close();
    if (!viable_start_exists) {
        err_msg <<
            "No valid start location possible. Check map file for errors.";
        throw std::runtime_error(err_msg.str());
    }

    // +0.5 to each dim to start in the center of designated grid square
    player_pos.x += 0.5;
    player_pos.y += 0.5;

    std::cout << "Parsed map file: " << map_filename << "\n";
}
