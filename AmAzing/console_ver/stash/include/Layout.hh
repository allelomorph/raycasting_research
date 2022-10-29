#ifndef LAYOUT_HH
#define LAYOUT_HH

#include <vector>
#include <string>
//#include "../include/Matrix.hh"  // Vector2d

// used Eigen::Vector2d

struct Layout {
    std::vector<std::vector<int>> map;
    uint32_t columns;
    uint32_t rows;

    Layout(std::string filename/*, Vector2d& pos*/);
};

#endif  // LAYOUT_HH
