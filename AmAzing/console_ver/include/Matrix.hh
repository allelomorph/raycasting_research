#ifndef MATRIX_HH
#define MATRIX_HH

#include <array>
#include <valarray>
#include <iostream>

#include <cstdint>   // uint32_t
#include <cassert>


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

    /*
    template <typename IntegralType>
    inline auto indexFromCoords(
        IntegralType row_i, IntegralType col_i) -> typename std::enable_if<
            std::is_integral<IntegralType>, IntegralType>::type {
        return (row_i * column_ct) + col_i;
    }
    */
public:
    //   https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html

    template<typename FriendScalarType,
             uint32_t FriendColumnSize, uint32_t FriendRowSize>
    friend class Matrix;

    // constructors
    Matrix() {}

    // TBD: does not protect against non-convertible types
    // https://www.appsloveworld.com/cplus/100/38/templates-how-to-control-number-of-constructor-args-using-template-variable
    template <typename... ConstructorParamTypes>
    Matrix(typename std::enable_if<
           sizeof...(ConstructorParamTypes) + 1 == RowSize * ColumnSize, ScalarType>::type param,
           ConstructorParamTypes... params)
        : array_repr { param, ScalarType(params)... } {}

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
    using CompatibleVector = Matrix<ScalarType, ColumnSize, 1>;
    CompatibleVector operator*(const CompatibleVector& vec) {
        CompatibleVector product;
        for (uint32_t row_i { 0 }; row_i < row_ct; ++row_i) {
            std::valarray<ScalarType> row_va(column_ct);
            for (uint32_t col_i { 0 }; col_i < column_ct; ++col_i) {
                row_va[col_i] = array_repr[(row_i * column_ct) + col_i] *
                    vec.array_repr[row_i];
            }
            product.array_repr[row_i] = row_va.sum();
        }
        return product;
    }

    // TBD: (?) create operator[] that returns a row subarray of array_repr, or member if Vector (RowSize == 1)

    // operator<<(scalar)
    //    mulitple terms with << popoulate matrix in level order
    // https://eigen.tuxfamily.org/dox/classEigen_1_1DenseBase.html#a0e575eb0ba6cc6bc5f347872abd8509d:
    // Original syntax is Matrix << val1, val2...;
    // This means we need an iterator, an operator,(), and a commainitializer class

// operator+(Vector2d)
/*
https://eigen.tuxfamily.org/dox/classEigen_1_1VectorwiseOp.html#a713694459d81b76e4f2a78e4d169f8d6
https://www.khanacademy.org/math/precalculus/x9e81a4f98389efdf:matrices/x9e81a4f98389efdf:properties-of-matrix-addition-and-scalar-multiplication/a/properties-of-matrix-addition
https://byjus.com/maths/matrix-addition/
*/
    /*
    template <typename MatrixType>
    Matrix Matrix::operator+(const MatrixType& m) {
        if (matrix[0].size() != m.matrix[0].size() ||
            matrix.size() != m.matrix.size()) {
            throw std::range_error(
                "Cannot add two Matrix objects of different orders (ColumnSize and RowSize)");
        }
        // TBD: return new Matrix that is modified version of this, different from +=
        for (uint32_t i {0}; i < matrix.size(); ++i) {
            for (uint32_t j {0}; j < matrix[0].size(); ++j) {
                matrix[i][j] += m.matrix[i][j];
            }
        }
    }
    */

    // index in array representation
    template <typename AccessIndexType>
    auto operator()(const AccessIndexType &s) -> typename std::enable_if<
        std::is_integral<AccessIndexType>::value, scalar_type>::type {
        assert(s > 0 && s < (AccessIndexType)array_repr.size());
        return array_repr[s];
    }

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

    template<typename _ScalarType, uint32_t _ColumnSize, uint32_t _RowSize>
    friend std::ostream& operator<<(std::ostream& os,
                         const Matrix<_ScalarType, _ColumnSize, _RowSize>& m);
};

template<typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
std::ostream& operator<<(std::ostream& os,
                         const Matrix<ScalarType, ColumnSize, RowSize>& m) {
    os << '{';
    for (uint32_t row_i { 0 }; row_i < m.row_ct; ++row_i) {
        os << '{';
        for (uint32_t col_i { 0 }; col_i < m.column_ct; ++col_i) {
            os << m.array_repr[(row_i * m.column_ct) + col_i];
            if (col_i < m.column_ct - 1)
                os << ", ";
        }
        os << '}';
        if (row_i < m.row_ct - 1)
            os << ", ";
    }
    return os << '}';
}

// Vector types are _column_ vectors
using Vector2i = Matrix<int, 2, 1>;
using Vector2d = Matrix<double, 2, 1>;

using Matrix2d = Matrix<double, 2, 2>;


#endif  // MATRIX_HH
