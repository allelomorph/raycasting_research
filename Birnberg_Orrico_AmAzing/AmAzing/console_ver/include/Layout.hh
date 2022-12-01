#ifndef LAYOUT_HH
#define LAYOUT_HH

#include "Matrix.hh"  // Vector2d

#include <cstdint>    // uint16_t

#include <vector>
#include <string>


struct Layout {
    std::vector<std::vector<int>> map;

    uint16_t cols;  // w
    uint16_t rows;  // h

    Layout(std::string filename, Vector2d& pos);

    inline bool coordsInsideWall(uint16_t x, uint16_t y) {
        return (map[y][x] != 0);
    }
};

#endif  // LAYOUT_HH
