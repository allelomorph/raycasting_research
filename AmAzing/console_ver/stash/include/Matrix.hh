#ifndef MATRIX_HH
#define MATRIX_HH

// Vector2d Vector2i
// uses Eigen::Vector2d::operator<<(scalar) (mulitple terms popoulate matrix in level order)
// uses Eigen::Vector2d, Eigen::Vector2i,
//   Eigen::Vector2d::operator+(Eigen::Vector2d),
//   Eigen::Vector2d::operator*(scalar),
//   Eigen::Vector2d::operator()(scalar)
//   Eigen::Vector2d::minCoeff(IndexType *index)
//      finds the minimum of all coefficients of *this and puts in *index its location
//   Eige::Matrix::cwise*: https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
//   Eigen::Vector2d::cwiseAbs()
//      std::abs(a[i]);
//   Eigen::Vector2d::cwiseInverse()
//      a[i] = 1 / a[i];
// uses Eigen::Vector2d Eigen::Matrix2d
//   Eigen::Matrix2d::operator<<(scalar)
//   Eigen::Matrix2d::operator*(Eigen::Vector2d)

#endif  // MATRIX_HH
