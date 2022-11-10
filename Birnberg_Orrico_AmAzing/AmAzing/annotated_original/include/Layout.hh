#ifndef LAYOUT_HH
#define LAYOUT_HH


#include "Matrix.hh"  // Vector2d

#include <cstdint>    // uint32_t

#include <vector>
#include <string>


struct Layout {
    std::vector<std::vector<int>> map;
    uint32_t columns;
    uint32_t rows;

    // TBD: can layout reinit be done separately from state reinit?
    Layout(std::string filename, Vector2d& pos);
};


#endif  // LAYOUT_HH
