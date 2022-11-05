#ifndef MATRIX_HH
#define MATRIX_HH

#include <array>
#include <valarray>
#include <iostream>

#include <cstdint>   // uint32_t
#include <cassert>
//#include <iterator>   // forward_iterator_tag
//#include <cstddef>    // ptrdiff_t


// Vector2d Vector2i

// constructors:
// ../original/src/app.cpp:183:    Vector2i mapPos(state->pos(0), state->pos(1));
//     Vector2i(Vector2d(0) (double), Vector2d(1) (double))
// ../original/src/app.cpp:182:    Vector2d ray = state->dir + state->viewPlane * t;
//     Vector2d operator = Vector2d + (Vector2d * double)
// ../original/src/app.cpp:184:    Vector2d dDist = ray.cwiseAbs().cwiseInverse();
//     Vector2d = Vector2d
// ../original/src/app.cpp:297:    Matrix2d rotate;
//      Matrix2d()
// ../original/src/app.cpp:300:    return (rotate * vector);
//      Matrix2d operator*(Vector2d)
// uses Eigen::Vector2d::operator<<(scalar) (mulitple terms popoulate matrix in level order)
// uses Eigen::Vector2d, Eigen::Vector2i,
//   Eigen::Vector2d::operator+(Eigen::Vector2d),
//   Eigen::Vector2d::operator*(scalar),
//   Eigen::Vector2d::operator()(scalar)
//   Eigen::Vector2d::minCoeff(IndexType *index)
//      finds the minimum of all coefficients of *this and puts in *index its location
//   Eigen::Matrix::cwise*: https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
//   Eigen::Vector2d::cwiseAbs()
//      std::abs(a[i]);
//   Eigen::Vector2d::cwiseInverse()
//      a[i] = 1 / a[i];
// uses Eigen::Vector2d Eigen::Matrix2d
//   Eigen::Matrix2d::operator<<(scalar)
//   Eigen::Matrix2d::operator*(Eigen::Vector2d)

// forward declaration of class and friends needed for templated friends of templated classes
/*
template<typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix;

template<typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
std::ostream& operator<<(std::ostream& os,
                         const Matrix<ScalarType, ColumnSize, RowSize>& m);
*/

// https://eigen.tuxfamily.org/dox/classEigen_1_1Matrix.html
// template<typename Scalar_, int Rows_, int Cols_, int Options_, int MaxRows_, int MaxCols_>
// class Eigen::Matrix< Scalar_, Rows_, Cols_, Options_, MaxRows_, MaxCols_ >

template<typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix {
private:
    // TBD: consider converting to valarray
    // array representation (left to right, top to bottom matrix traversal)
    std::array<ScalarType, RowSize * ColumnSize> array_repr;

    uint32_t row_ct    { ColumnSize };
    uint32_t column_ct { RowSize };
    using scalar_type = ScalarType;

public:
    // constructors
    // operator<<(scalar)
    //    mulitple terms with << popoulate matrix in level order
    // operator+(Vector2d)

    // operator*(scalar)
    // operator*(Vector2d)
    // operator()(scalar)
    // minCoeff(IndexType *index)
    //   finds the minimum of all coefficients of *this and puts in *index its location
    // cwise*:
    //   https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
    // cwiseAbs()
    //   std::abs(a[i]) for a in matrix
    // cwiseInverse()
    //   a[i] = 1 / a[i] for a in matrix

    template<typename FriendScalarType,
             uint32_t FriendColumnSize, uint32_t FriendRowSize>
    friend class Matrix;

    // constructors
    Matrix() {}

    // TBD: does not protect against non-convertible types
    template <typename... ConstructorParamTypes>
    Matrix(typename std::enable_if<
           sizeof...(ConstructorParamTypes) + 1 == RowSize * ColumnSize, ScalarType>::type param,
           ConstructorParamTypes... params)
        : array_repr { param, ScalarType(params)... } {}
    /*
    template <typename... Tail>
    Matrix(typename std::enable_if<
           sizeof...(Tail) + 1 == RowSize * ColumnSize, ScalarType>::type head, Tail... tail)
        : array_repr { head, ScalarType(tail)... } {}
    */
    /*
    template <typename... Tail>
    Matrix(typename std::enable_if<
           sizeof...(Tail) + 1 <= RowSize * ColumnSize, ScalarType>::type head, Tail... tail)
        : array_repr { head, ScalarType(tail)... } {}
    */
    /*
    template <typename... ConstructorParamTypes>
    Matrix(ScalarType param, ConstructorParamTypes... params) {
        static_assert(sizeof...(ConstructorParamTypes) + 1 <= RowSize * ColumnSize,
                      "more Matrix constuctor parameters than coefficients in Matrix type");
        array_repr = decltype(array_repr) { param, ScalarType(params)... };
    }
    */


    scalar_type* begin() { return array_repr.begin(); }
    scalar_type* end()   { return array_repr.end(); }

    template <typename ScalarMultiplierType>
    auto operator*(const ScalarMultiplierType& sm) -> typename std::enable_if<
        std::is_scalar<ScalarMultiplierType>::value, Matrix>::type {
        Matrix product { *this };
        for (auto &coeff: product.array_repr)
            coeff *= sm;
        return product;
    }

    // simplify from matrix * matrix to matrix * vector due to size of return matrix being dependent on order of largest
    // can be futher simplified to enforce multiplier of same scalar type: Matrix2d * Vector2d =
    //   Matrix<double, 2, 2> * Matrix<double, 2, 1>
    // TBD: currently operates on matching scalar types
    Matrix<ScalarType, ColumnSize, 1> operator*(
        const Matrix<ScalarType, ColumnSize, 1>& vec) {
        Matrix<ScalarType, ColumnSize, 1> product;
        for (uint32_t row_i { 0 }; row_i < row_ct; ++row_i) {
            std::valarray<ScalarType> row_va(column_ct);
            for (uint32_t col_i { 0 }; col_i < column_ct; ++col_i)
                row_va[col_i] = array_repr[(row_i * row_ct) + col_i] * vec.array_repr[row_i];
            product.array_repr[row_i] = row_va.sum();
        }
        return product;
    }
/*
    template <typename MultiplierScalarType>
    Matrix<MultiplierScalarType, ColumnSize, 1> operator*(
        const Matrix<MultiplierScalarType, ColumnSize, 1>& vec) {
        Matrix<MultiplierScalarType, ColumnSize, 1> product;
        for (uint32_t row_i { 0 }; row_i < row_ct; ++row_i) {
            std::valarray<decltype(array_repr[0] * vec.array_repr[0])> row_va(column_ct);
            for (uint32_t col_i { 0 }; col_i < column_ct; ++col_i)
                row_va[col_i] = array_repr[(row_i * row_ct) + col_i] * vec.array_repr[row_i];
            product.array_repr[row_i] = row_va.sum();
        }
        return product;
    }
*/

    // TBD: create operator[] that returns a row subarray of array_repr, or member if Vector (RowSize == 1)

    // like operator[], but in array representation
    template <typename AccessIndexType>
    auto operator()(const AccessIndexType &s) -> typename std::enable_if<
        std::is_integral<AccessIndexType>::value, scalar_type>::type {
        assert(s > 0 && s < (AccessIndexType)array_repr.size());
        return array_repr[s];
    }

    // finds the minimum of all coefficients of *this and puts in *index its location
    template <typename MinCoeffIndexType>
    auto minCoeff(MinCoeffIndexType *index) -> typename std::enable_if<
        std::is_integral<MinCoeffIndexType>::value, void>::type {
        *index = *(std::min_element(array_repr.begin(), array_repr.end()));
    }

    void cwiseAbs() {
        for (auto &coeff : array_repr)
            coeff = std::abs(coeff);
    }

    void cwiseInverse()  {
        for (auto &coeff : array_repr)
            coeff = 1 / coeff;
    }

    /*
    friend std::ostream& operator<<(std::ostream& os,
                         const Matrix<ScalarType, ColumnSize, RowSize>& m);
    */
};

/*
template<typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
std::ostream& operator<<(std::ostream& os,
                         const Matrix<ScalarType, ColumnSize, RowSize>& m) {
    os << '{';
    for (uint32_t row_i { 0 }; row_i < m.row_ct; ++row_i) {
        os << '{';
        for (uint32_t col_i { 0 }; col_i < m.column_ct; ++col_i) {
            os << m.array_repr[(row_i * m.row_ct) + col_i];
            if (col_i < m.column_ct - 1)
                os << ", ";
        }
        os << '}';
        if (row_i < m.row_ct - 1)
            os << ", ";
    }
    return os << '}';
}
*/
// Vector* types are _column_ vectors
using Vector2i = Matrix<int, 2, 1>;
using Vector2d = Matrix<double, 2, 1>;

using Matrix2d = Matrix<double, 2, 2>;


#endif  // MATRIX_HH
