/*
 * @file Vector2d.hh was stripped down from Matrix.hh to improve the
 *   performance and readability of Vector2d use in monoplanar_demo, at the
 *   cost of Matrix's templated flexbility.
 */

#ifndef VECTOR2D_HH
#define VECTOR2D_HH


#include <cmath>        // M_PI

#include <type_traits>  // enable_if


class Vector2d {
private:
    // https://stackoverflow.com/questions/6247153/angle-from-2d-unit-vector
    static constexpr inline double radiansToDegrees(const double radians) {
        return radians * (180 / M_PI);
    }

public:
    double x;
    double y;

    Vector2d() {}
    Vector2d(const double _x, const double _y) : x(_x), y(_y) {}

    template <typename ScalarMultiplierType>
    auto operator*(const ScalarMultiplierType sm) -> typename std::enable_if<
        std::is_scalar<ScalarMultiplierType>::value, Vector2d>::type {
        return Vector2d { this->x * sm, this->y * sm };
    }

    Vector2d operator+(const Vector2d other);
    // degrees from +x axis
    double angle() const;
    // ccw rotation (QI to QIV)
    void rotate(const double radians);
};


#endif  // VECTOR2D_HH
