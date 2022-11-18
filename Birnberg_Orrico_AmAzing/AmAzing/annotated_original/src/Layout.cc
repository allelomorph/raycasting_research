#include "Layout.hh"
#include "Matrix.hh"  // Vector2d Vector2d::operator()

#include <cstdlib>
#include <cstdint>    // uint32_t
#include <cctype>     // isdigit

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>     // getline


// constructor parses map file into unit grid map for raycasting
// character rows and columns in map file correspond to grid positions
// 0 denotes empty grid coordinate, non-0 is wall unit
// x in map file denotes starting character position (last one to be parsed is used)
// 1-8 denote wall texture
Layout::Layout(std::string filename, Vector2d& pos) {
    std::ostringstream err_msg;
    std::ifstream map_ifs(filename);
    if (!map_ifs.is_open()) {
        err_msg << "Could not open file: " << filename << std::endl;
        throw std::runtime_error(err_msg.str());
    }

    std::string line;
    bool valid_start_exists { false };
    rows = 0;     // this->rows
    columns = 0;  // this->columns
    for (std::vector<int> row; std::getline(map_ifs, line);) {
        ++rows;
        row.clear();
        uint32_t i;
        for (i = 0; i < line.size(); ++i) {
            // empty map grid unit
            if (!valid_start_exists && line[i] == '0') {
                pos(0) = rows - 1;
                pos(1) = i;
                valid_start_exists = true;
            }
            // TBD: currently does not check for multiple starts
            // player start position
            if (line[i] == 'x') {
                valid_start_exists = true;
                pos(0) = rows - 1;
                pos(1) = i;
            }
            // TBD: no error on '9', despite missing wall texture
            // map wall unit (wall texture corresponds to digit 1-8)
            if (!std::isdigit(line[i]))
                line[i] = '0';
            row.push_back(line[i] - '0');
        }
        // ensure grid map can accommodate widest row in map file
        if (i > columns)
            columns = i;
        map.push_back(row);  // this->map
    }

    map_ifs.close();
    if (!valid_start_exists) {
        err_msg << "No valid start location possible! Check your map!!" << std::endl;
        throw std::runtime_error(err_msg.str());
    }

    // add outer wall (texture 1) to east and west
    for (auto &row : map) {
        row.resize(columns, 0);
        row.insert(row.begin(), 1);
        row.push_back(1);
    }

    // add outer wall (texture 2) to east and west
    rows += 2;
    columns += 2;
    map.insert(map.begin(), std::vector<int>(columns, 1));
    map.push_back(std::vector<int>(columns, 1));

    // TBD: are these position updates to compensate for map resize?
    pos(0) += 1.5;
    pos(1) += 1.5;
}
