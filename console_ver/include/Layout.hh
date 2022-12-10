#ifndef LAYOUT_HH
#define LAYOUT_HH

#include "Matrix.hh"  // Vector2d

#include <cstdint>    // uint16_t

#include <vector>
#include <string>


struct Layout {
    // TBD: does this need to be a 4-byte int?
    std::vector<std::vector<int>> map;

    // TBD: rename these and move map_filename here (curr_map_filename?)
    uint16_t cols;  // w
    uint16_t rows;  // h

    // TBD: separate cosntructor from loadMapFile(file, pos)
    Layout(std::string filename, Vector2d& pos);

    inline bool coordsInsideWall(const uint16_t x, const uint16_t y) const {
        return (map[y][x] != 0);
    }
};

#endif  // LAYOUT_HH
