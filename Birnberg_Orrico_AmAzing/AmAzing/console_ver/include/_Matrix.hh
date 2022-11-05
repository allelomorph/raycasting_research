#ifndef MATRIX_HH
#define MATRIX_HH

#include <array>
#include <valarray>
#include <iostream>

#include <cstdint>   // uint32_t
#include <cassert>


// https://stackoverflow.com/questions/17075506/operator-on-comma-separated-values-in-c
// TBD: wrap in namespace to obscure user access to class
/** @brief  Comma-separated Matrix Initializer

 The class instances are usually not created explicitly.
 Instead, they are created on "matrix << firstValue" operator.

 The sample below initializes 2x2 rotation matrix:

 \code
 double angle = 30, a = cos(angle*CV_PI/180), b = sin(angle*CV_PI/180);
 Mat R = (Mat_<double>(2,2) << a, -b, b, a);
 \endcode
*/
/*
template<typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class MatCommaInitializer {
public:
    using matrix_type = Matrix<ScalarType, ColumnSize, RowSize>;
    using matrix_type::iterator_type;

    // constructor, created by "matrix << firstValue" operator, where matrix is cv::Mat
    MatCommaInitializer(matrix_type& mat, iterator_type iter) : it {iter}, matrix {mat} {}

    // operator that takes the next value and put into the matrix
    template<typename ScalarInputType>
    MatCommaInitializer& operator,(ScalarInputType v) {
        assert( it < matrix.end() );
        *it = ScalarType(v);
        ++it;
        return *this;
    }

    // another form of conversion operator
    // direct cst operator (?)
    // operator matrix_type() const;
private:
    matrix_type& matrix;
    iterator_type it;
};
*/
/*
template<typename _Tp> inline
MatCommaInitializer_<_Tp>::operator Mat_<_Tp>() const
{
    assert( this->it == ((const Mat_<_Tp>*)this->it.m)->end() );
    return Mat_<_Tp>(*this->it.m);
}
*/
/*
template <typename MatrixScalarType, uint32_t ColumnSize, uint32_t RowSize,
          typename InputScalarType>
static inline MatCommaInitializer<_Tp> operator<<(
    const Matrix<MatrixScalarType, ColumnSize, RowSize>& m, InputScalarType val) {
    MatCommaInitializer<MatrixScalarType, ColumnSize, RowSize> comma_initializer(m.begin());
    return (comma_initializer, val);
}
*/

// https://eigen.tuxfamily.org/dox/classEigen_1_1Matrix.html
// template<typename Scalar_, int Rows_, int Cols_, int Options_, int MaxRows_, int MaxCols_>
// class Eigen::Matrix< Scalar_, Rows_, Cols_, Options_, MaxRows_, MaxCols_ >

template <typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix {
private:
    // TBD: consider converting to valarray
    // array representation (left to right, top to bottom matrix traversal)
    std::array<ScalarType, RowSize * ColumnSize> array_repr;

    uint32_t row_ct     { ColumnSize };
    uint32_t column_ct  { RowSize };

public:
    using scalar_type   = ScalarType;
    using iterator_type = decltype(array_repr.begin());

    // forward declare to first complete definition of Matrix
    class MatrixCommaInitializer;

    // https://stackoverflow.com/questions/8569029/c-is-it-possible-to-friend-all-instances-of-a-template-class
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

    // https://cse.buffalo.edu/~erdem/cse331/support/matrix-vect/
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

    // https://stackoverflow.com/questions/17075506/operator-on-comma-separated-values-in-c
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
    auto operator()(const AccessIndexType &s) const -> typename std::enable_if<
        std::is_integral<AccessIndexType>::value, scalar_type>::type {
        assert(s > 0 && s < (AccessIndexType)array_repr.size());
        return array_repr[s];
    }

    // https://www.eigen.tuxfamily.org/dox/classEigen_1_1DenseBase.html#a0739f9c868c331031c7810e21838dcb2
    template <typename MinCoeffIndexType>
    auto minCoeff(MinCoeffIndexType *index) const -> typename std::enable_if<
        std::is_integral<MinCoeffIndexType>::value, void>::type {
        *index = *(std::min_element(array_repr.begin(), array_repr.end()));
    }

    // https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
    void cwiseAbs() {
        for (auto &coeff : array_repr)
            coeff = std::abs(coeff);
    }

    // https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
    void cwiseInverse()  {
        for (auto &coeff : array_repr)
            coeff = 1 / coeff;
    }

    // TBD: why are these declarations acting differnently?
    // https://stackoverflow.com/questions/4039817/friend-declaration-declares-a-non-template-function
    // https://www.ibm.com/docs/en/zos/2.3.0?topic=only-friends-templates-c
    template<typename _ScalarType, uint32_t _ColumnSize, uint32_t _RowSize>
    friend std::ostream& operator<<(
        std::ostream& os, const Matrix<_ScalarType, _ColumnSize, _RowSize>& m);

/*
    template <typename InputScalarType>
    friend MatrixCommaInitializer operator<<(Matrix& m, InputScalarType val);
*/
    template<typename _FriendScalarType,
             uint32_t _FriendColumnSize, uint32_t _FriendRowSize>
    friend MatrixCommaInitializer operator<<(Matrix& m, double val);

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


template <typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix<ScalarType, ColumnSize, RowSize>::MatrixCommaInitializer {
public:
    //using matrix_type = Matrix/*<ScalarType, ColumnSize, RowSize>*/;
    using iterator_type = decltype(Matrix().begin());
    //using iterator_type = Matrix<ScalarType, ColumnSize, RowSize>::iterator_type;
    //using Matrix::iterator_type;

    // constructor, created by "matrix << firstValue" operator
    MatrixCommaInitializer(Matrix& mat, iterator_type iter) :
        matrix {mat}, it {iter} {}

    // operator that takes the next value and put into the matrix
    template<typename ScalarInputType>
        auto operator,(ScalarInputType v) -> typename std::enable_if<
            std::is_scalar<ScalarInputType>::value, MatrixCommaInitializer&>::type {
        assert( it < matrix.end() );
        *it = ScalarType(v);
        ++it;
        return *this;
    }

private:
    Matrix& matrix;
    iterator_type it;
};


template <typename ScalarType, uint32_t ColumnSize, uint32_t RowSize,
          typename InputScalarType>
auto operator<<(
    Matrix<ScalarType, ColumnSize, RowSize>& m, InputScalarType val) -> typename std::enable_if<
    std::is_scalar<InputScalarType>::value,
    typename Matrix<ScalarType, ColumnSize, RowSize>::MatrixCommaInitializer>::type {
    typename Matrix<
        ScalarType, ColumnSize, RowSize>::MatrixCommaInitializer comma_initializer(m, m.begin());
    return (comma_initializer, val);
}
/*
Matrix<double, 2, 2>::MatrixCommaInitializer operator<<(Matrix<double, 2, 2>& m, double val) {
    Matrix<double, 2, 2>::MatrixCommaInitializer comma_initializer(m, m.begin());
    return (comma_initializer, val);
}
*/

#endif  // MATRIX_HH
