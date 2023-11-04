#ifndef LAYOUT_HH
#define LAYOUT_HH

#include "Vector2d.hh"

#include <cstdint>    // uint16_t
// #include <cassert>

#include <vector>
#include <string>


struct Layout {
private:
    // Originally a 2D vector to aid in map file parsing, converted to 1D vector
    //   with coordinate access similar to TtyScreenBuffer::pixels, as from
    //   profiling, map access with tile() and tileIsWall() seems to be the
    //   next worst performance bottleneck after TtyScreenBuffer::pixel().
    // If coordinates in a human-readable map file can be considered to start
    //   at (0,0) in the upper left and increase for x in left-to-right and y in
    //   top-to-bottom order, respectively. But once parsed, map data
    //   represents a Quadrant I coordinate grid in the raycasting engine, so
    //   rows are stored in reversed order so +y always goes "north" in the map.
    std::vector<uint8_t> map;

public:
    uint16_t w;  // cols
    uint16_t h;  // rows

    void resize(const uint16_t _w, const uint16_t _h) {
        w = _w;
        h = _h;
        map.resize(w * h);
    }

    uint8_t& tile(const uint16_t x, const uint16_t y) {
        // assert(x < w && y < h);
        return map[(y * w) + x];
    }

    const uint8_t& tile(const uint16_t x, const uint16_t y) const {
        // assert(x < w && y < h);
        return map[(y * w) + x];
    }

    bool tileIsWall(const uint16_t x, const uint16_t y) const {
        // assert(x < w && y < h);
        return map[(y * w) + x] != 0;
    }

    // parses map file in with inverted rows
    void loadMapFile(const std::string& map_filename, Vector2d& player_pos);
};


#endif  // LAYOUT_HH
