#include "../include/Layout.hh"
#//include "../include/Matrix.hh"
#include <iostream>
#include <fstream>

// used Eigen::Vector2d, Eigen::Vector2d::operator()

// parsing map file into map in memory
Layout::Layout(std::string filename/*, Vector2d& pos*/) {
    std::ifstream map_file(filename);
    if (!map_file.is_open()) {
        std::cout << "Could not open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string line;
    bool viable_start_exists = false;
    bool chosen_start_exists = false;
    rows = 0;
    columns = 0;
    std::vector<int> row;
    std::cout << "parsing map file:\n";
    while (std::getline(map_file, line) && line.size()) {
        ++rows;
        std::cout << line << '\n';
        for (uint32_t i {0}; line[i] != '\0'; ++i) {
            if (line[i] == 'x') {
                if (chosen_start_exists) {
                    std::cout <<
                        "Mulitple start points specified. Check your map!!" << std::endl;
                    exit(EXIT_FAILURE);
                }
                chosen_start_exists = true;
                viable_start_exists = true;
                //pos(0) = rows - 1;
                //pos(1) = i;
                row.emplace_back(0);
            } else if (std::isdigit(line[i])) {
                // default start is first empty map tile found
                //   (most northern, then eastern)
                if (!viable_start_exists && line[i] == '0') {
                    viable_start_exists = true;
                    //pos(0) = rows - 1;
                    //pos(1) = i;
                }
                // TBD: C++ idiomatic conversion of digit char?
                row.emplace_back(line[i] - '0');
            } else if (line[i] == ' ') {  // \t width unpredictable
                // pad out non-rectangular western maze edges on map
                row.emplace_back(1);
            } else {
                std::cout <<
                    "Unrecognized character in map encoding. Check your map!!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        // columns set to max width of variable width maps
        columns = std::max((uint32_t)line.size(), columns);
        map.push_back(row);
        row.clear();
    }
    map_file.close();
    if (!viable_start_exists) {
        std::cout <<
            "No valid start location possible! Check your map!!" << std::endl;
        exit(EXIT_FAILURE);
    }
    for (auto & row: map) {
        // pad out non-rectangular eastern maze edges on map
        if (row.size() < columns)
            row.resize(columns, 1);
        // add outer E and W walls (even if already bounded)
        row.insert(row.begin(), 1);
        row.push_back(1);
    }
    columns += 2;
    // add outer N and S walls (even if already bounded)
    map.insert(map.begin(), std::vector<int>(columns, 1));
    map.push_back(std::vector<int>(columns, 1));
    rows += 2;
    //pos(0) += 1.5;
    //pos(1) += 1.5;
    std::cout << "parsed map:\n";
    for (const auto &row : map) {
        for (const auto &tile : row)
            std::cout << tile;
        std::cout << "\n";
    }
}
