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

template <typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix {
private:
    // TBD: consider converting to valarray
    // array representation (left to right, top to bottom matrix traversal)
    std::array<ScalarType, RowSize * ColumnSize> array_repr;

    uint32_t row_ct     { ColumnSize };
    uint32_t column_ct  { RowSize };

    /*
    inline uint32_t arrayIndex(uint32_t row_i, uint32_t col_i) {
        return (row_i * column_ct) + col_i;
    }
    */

    // Matrix members used in CommaInitializer definition, need to forward declare
    class CommaInitializer;

public:
    using scalar_type   = ScalarType;
    using iterator_type = decltype(array_repr.begin());


    // https://stackoverflow.com/questions/8569029/c-is-it-possible-to-friend-all-instances-of-a-template-class
    template<typename FriendScalarType,
             uint32_t FriendColumnSize, uint32_t FriendRowSize>
    friend class Matrix;

    // constructors
    Matrix() {
        static_assert(ColumnSize > 1 && RowSize > 0,
                      "Matrix height minimum of 2 and width minimum of 1 (Vector)");
    }

    // Seems to enforce uniform param type in that next upacked parameter is ScalarType
    // https://www.appsloveworld.com/cplus/100/38/templates-how-to-control-number-of-constructor-args-using-template-variable
    template <typename... ConstructorParamTypes>
    Matrix(typename std::enable_if<
           (ColumnSize > 1 && RowSize > 0) &&
           sizeof...(ConstructorParamTypes) + 1 == RowSize * ColumnSize, ScalarType>::type param,
           ConstructorParamTypes... params)
        : array_repr { param, ScalarType(params)... } {
    }

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
    // https://www.khanacademy.org/math/precalculus/x9e81a4f98389efdf:matrices/x9e81a4f98389efdf:multiplying-matrices-by-matrices/a/multiplying-matrices?modal=1
    // can be futher simplified to enforce multiplier of same scalar type: Matrix2d * Vector2d =
    // TBD: currently only designed for Matrix<double, 2, 2> * Matrix<double, 2, 1>
    //   Could be expanded to take differing scalar types, or generalized to take
    //   matrix trather than vector multipliers, but that requires a variable return
    //   type as the width of the product has to match the greatest operand width
    // TBD: currently heights need to match - can this be further generalized?
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

    // Matrix(index in array representation)
    template <typename ParenthesesIndexType>
    auto operator()(const ParenthesesIndexType &s) -> typename std::enable_if<
        std::is_integral<ParenthesesIndexType>::value, scalar_type&>::type {
        assert(s >= 0 && s < (ParenthesesIndexType)array_repr.size());
        return array_repr[s];
    }

    // https://www.eigen.tuxfamily.org/dox/classEigen_1_1DenseBase.html#a0739f9c868c331031c7810e21838dcb2
    template <typename MinCoeffIndexType>
    auto minCoeff(MinCoeffIndexType *index) -> typename std::enable_if<
        std::is_integral<MinCoeffIndexType>::value, void>::type {
        *index = std::min_element(array_repr.begin(), array_repr.end()) -
            array_repr.begin();
    }

    // https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
    Matrix cwiseAbs() {
        Matrix result { *this };
        for (auto &coeff : result.array_repr)
            coeff = std::abs(coeff);
        return result;
    }

    // https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
    Matrix cwiseInverse()  {
        Matrix result { *this };
        for (auto &coeff : result.array_repr)
            coeff = 1 / coeff;
        return result;
    }

    // https://stackoverflow.com/questions/4039817/friend-declaration-declares-a-non-template-function
    // https://www.ibm.com/docs/en/zos/2.3.0?topic=only-friends-templates-c
    // Defining templated friend functions inside a templated class saves on
    //   verbosity, see:
    //   - https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Making_New_Friends
    friend std::ostream& operator<<(std::ostream& os,
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

    template <typename InputScalarType>
    using CommaInit_return_if_scalar_input = typename std::enable_if<
        std::is_scalar<InputScalarType>::value,
        typename Matrix<ScalarType, ColumnSize, RowSize>::CommaInitializer>::type;

    // https://eigen.tuxfamily.org/dox/classEigen_1_1DenseBase.html#a0e575eb0ba6cc6bc5f347872abd8509d
    // https://stackoverflow.com/questions/17075506/operator-on-comma-separated-values-in-c

    template <typename InputScalarType>
    friend auto operator<<(Matrix<ScalarType, ColumnSize, RowSize>& m,
                           InputScalarType val) -> CommaInit_return_if_scalar_input<InputScalarType> {
        typename Matrix<
            ScalarType, ColumnSize, RowSize>::CommaInitializer comma_initializer(m, m.begin());
        return (comma_initializer, val);
    }
};

// adapted from OpenCV, which uses similar <</, overloads to Eigen
// https://github.com/opencv/opencv/blob/HEAD/modules/core/include/opencv2/core/mat.hpp#L523
// https://github.com/opencv/opencv/blob/HEAD/modules/core/include/opencv2/core/mat.inl.hpp#L2972
template <typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix<ScalarType, ColumnSize, RowSize>::CommaInitializer {
public:
    using iterator_type = Matrix::iterator_type;

    // constructor, created by `Matrix << first_scalar` operator
    CommaInitializer(Matrix& mat, iterator_type iter) :
        matrix {mat}, it {iter} {}

    // overloading comma operator allows Matrix population with `Matrix << v1, v2, ...`
    template<typename ScalarInputType>
        auto operator,(ScalarInputType v) -> typename std::enable_if<
            std::is_scalar<ScalarInputType>::value, CommaInitializer&>::type {
        assert( it < matrix.end() );
        *it = ScalarType(v);
        ++it;
        return *this;
    }

private:
    Matrix& matrix;
    iterator_type it;
};


// Vector types are _column_ vectors
using Vector2i = Matrix<int, 2, 1>;
using Vector2d = Matrix<double, 2, 1>;
using Matrix2d = Matrix<double, 2, 2>;


#endif  // MATRIX_HH
