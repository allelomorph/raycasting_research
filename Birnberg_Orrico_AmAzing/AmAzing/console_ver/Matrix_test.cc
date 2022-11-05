#include "Matrix.hh"

#include <iostream>
#include <valarray>

#include "typeName.hh"

int main() {
    Vector2i v2i (1, 2);
    Vector2d v2d (1.0, 2.0);
    Matrix2d m2d (1.1, 1.2, 2.1, 2.2);

    (void)v2i;
    Vector2d v2d2 (m2d * v2d);
    Matrix2d m2d2 (m2d * 2);

    for (auto &coeff : m2d)
        std::cout << coeff << ' ';
    std::cout << '\n';
    for (auto &coeff : m2d2)
        std::cout << coeff << ' ';
    std::cout << '\n';
    for (auto &coeff : v2d2)
        std::cout << coeff << ' ';
    std::cout << '\n';

    std::cout << m2d << '\n';
    std::cout << m2d2 << '\n';
    std::cout << v2d2 << '\n';

    m2d << 10.1, 10.2, 20.1, 20.2;
    std::cout << m2d << '\n';
}
