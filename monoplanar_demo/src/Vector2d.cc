#include "Vector2d.hh"

#include <cmath>  // atan M_PI


Vector2d Vector2d::operator+(const Vector2d other) {
    return Vector2d { this->x + other.x, this->y + other.y };
}

// https://stackoverflow.com/questions/6247153/angle-from-2d-unit-vector
double Vector2d::angle() const {
    double angle { radiansToDegrees(std::atan(this->y / this->x)) };
    if (this->x < 0)  // quadrant II or III
        angle = 180 + angle;  // subtracts
    else if (this->y < 0)  // quadrant IV
        angle = 270 + (90 + angle);  // subtracts
    return angle;
}

// counterclockwise (QI to QIV) for positive `radians`
// using 2d vector rotation matrix, see: https://www.cuemath.com/algebra/rotation-matrix/
void Vector2d::rotate(const double radians) {
    Vector2d rotation {
        std::cos(radians) * this->x - std::sin(radians) * this->y,
        std::sin(radians) * this->x + std::cos(radians) * this->y };
    *this = rotation;
}
