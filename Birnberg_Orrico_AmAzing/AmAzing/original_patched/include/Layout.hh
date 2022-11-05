#ifndef LAYOUT_HH
#define LAYOUT_HH

#include <vector>
#include <string>
#include <Dense>  // Eigen/Dense

// uses Eigen::Vector2d

struct Layout {
    std::vector<std::vector<int>> map;
    uint32_t columns;
    uint32_t rows;

    Layout(std::string filename, Eigen::Vector2d& pos);
};

#endif  // LAYOUT_HH
