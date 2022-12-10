#ifndef LAYOUT_HH
#define LAYOUT_HH

#include "Matrix.hh"  // Vector2d

#include <cstdint>    // uint16_t
#include <cassert>

#include <vector>
#include <string>


struct Layout {
private:
    // Index "coordinates" in a 2D array can be considered to start at 0,0 in
    //   the upper left and increase for x in left-to-right and y in
    //   top-to-bottom order, respectively. But as this is a representation of
    //   both the human-readable map file and a Quadrant I coordinate grid in
    //   the raycasting engine, rows are stored in reversed order so +y (and
    //   +row_i) always goes "north" in the map.
    using tile_type = uint8_t;
    std::vector<std::vector<tile_type>> map;

public:
    uint16_t w;  // cols
    uint16_t h;  // rows

    inline uint8_t& tileData(const uint16_t x, const uint16_t y) {
        assert(x < w && y < h);
        return map[y][x];
    }

    inline const uint8_t& tileData(const uint16_t x, const uint16_t y) const {
        assert(x < w && y < h);
        return map[y][x];
    }

    inline bool tileIsWall(const uint16_t x, const uint16_t y) const {
        assert(x < w && y < h);
        return (map[y][x] != 0);
    }

    void loadMapFile(const std::string& map_filename, Vector2d& player_pos);
};

#endif  // LAYOUT_HH
