/*
 * @file Matrix.hh was built as a learning exercise to substitute the original
 *   AmAzing's use of the Eigen library classes Matrix2d, Vector2d, Vector2i.
 *
 *   (As most definitions are templated, no Matrix.cc is provided.)
 */

#ifndef MATRIX_HH
#define MATRIX_HH


#include <valarray>
#include <iostream>

#include <cstdint>   // uint32_t
#include <cassert>


/*
 * @brief Matrix serves as a template for Matrix2d, Vector2d, Vector2i, and is
 *   loosely based on Eigen::Matrix, see:
 *   - https://eigen.tuxfamily.org/dox/classEigen_1_1Matrix.html
 */
template <typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix {
private:
    // array representation (left to right, top to bottom matrix traversal)
    std::valarray<ScalarType> data;

    // Matrix members used in CommaInitializer definition, need to forward declare
    class CommaInitializer;

public:
    uint32_t rows;
    uint32_t columns;
    using scalar_type   = ScalarType;
    using iterator_type = decltype(std::begin(data));

    // Friend all other instantialized versions of this template, see:
    //   - https://stackoverflow.com/questions/8569029/
    template<typename FriendScalarType,
             uint32_t FriendColumnSize, uint32_t FriendRowSize>
    friend class Matrix;

    /*
     * constructors
     */
    Matrix() : data ( RowSize * ColumnSize ),
               rows { ColumnSize }, columns { RowSize } {
        static_assert(ColumnSize > 1 && RowSize > 0,
                      "Matrix height minimum of 2 and width minimum of 1 (Vector)");
    }

    // https://www.appsloveworld.com/cplus/100/38/templates-how-to-control-number-of-constructor-args-using-template-variable
    template <typename... ConstructorParamTypes>
    Matrix(typename std::enable_if<
           (ColumnSize > 1 && RowSize > 0) && sizeof...(ConstructorParamTypes) + 1 == RowSize * ColumnSize,
           ScalarType>::type param, ConstructorParamTypes... params) :
        data { param, ScalarType(params)... },
        rows { ColumnSize }, columns { RowSize } {}

    /*
     * utilities
     */
    std::size_t size() { return data.size(); }

    ScalarType* begin() { return std::begin(data); }
    ScalarType* end()   { return std::end(data); }

    void clear(ScalarType value = ScalarType {}) { data = value; }

    /*
     * indexed access operators
     */
    ScalarType& operator()(const uint32_t i) {
        assert(i < rows * columns);
        return data[i];
    }

    const ScalarType& operator()(const uint32_t i) const {
        assert(i < rows * columns);
        return data[i];
    }

    ScalarType& operator()(const uint32_t x, const uint32_t y) {
        assert(x < columns && y < rows);
        return data[(columns * y) + x];
    }

    const ScalarType& operator()(const uint32_t x, const uint32_t y) const {
        assert(x < columns && y < rows);
        return data[(columns * y) + x];
    }

    std::slice_array<ScalarType> row(uint32_t row_i) {
        return data[std::slice(columns * row_i, columns, 1)];
    }

    std::slice_array<ScalarType> column(uint32_t column_i) {
        return data[std::slice(column_i, rows, columns)];
    }

    /*
     * @brief multiplication by scalar
     */
    template <typename ScalarMultiplierType>
    auto operator*(const ScalarMultiplierType& sm) -> typename std::enable_if<
        std::is_scalar<ScalarMultiplierType>::value, Matrix>::type {
        Matrix result { *this };
        result.data *= ScalarType(sm);
        return result;
    }

    /*
     * @brief multiplication by Vector
     *
     * @notes TBD: currently designed for its only use in AmAzing:
     *   Matrix<double, 2, 2> * Matrix<double, 2, 1>. Template could be
     *   expanded to take differing scalar types, or generalized to take
     *   matrix trather than vector multipliers, but that requires a variable
     *   return type, as the width of the product matrix has to match the
     *   larger operand width (heights must always match,) see:
     *   - https://cse.buffalo.edu/~erdem/cse331/support/matrix-vect/
     *   - https://www.khanacademy.org/math/precalculus/x9e81a4f98389efdf:matrices/x9e81a4f98389efdf:multiplying-matrices-by-matrices/a/multiplying-matrices?modal=1
     */
    using CompatibleVector = Matrix<ScalarType, ColumnSize, 1>;
    CompatibleVector operator*(const CompatibleVector& vec) {
        CompatibleVector product;
        for (uint32_t row_i { 0 }; row_i < rows; ++row_i) {
            std::valarray<ScalarType> row_ ( row(row_i) );
            product.data[row_i] = (row_ * vec.data[row_i]).sum();
        }
        return product;
    }

    /*
     * @brief Find minimum coefficient and place its index in the provided pointer
     *
     * @notes analog for: - https://www.eigen.tuxfamily.org/dox/classEigen_1_1DenseBase.html#a0739f9c868c331031c7810e21838dcb2
     */
    template <typename MinCoeffIndexType>
    auto minCoeff(MinCoeffIndexType* index) -> typename std::enable_if<
        std::is_integral<MinCoeffIndexType>::value, void>::type {
        *index = std::min_element(begin(), end()) - begin();
    }

    /*
     * @brief Returns Matrix with absolute values of original coefficients
     *
     * @notes See Eigen original at: - https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
     */
    Matrix cwiseAbs() {
        Matrix result { *this };
        result.data = std::abs(result.data);
        return result;
    }

    /*
     * @brief Returns Matrix with inverse values of original coefficients
     *
     * @notes See Eigen original at: - https://www.eigen.tuxfamily.org/dox/group__CoeffwiseMathFunctions.html
     */
    Matrix cwiseInverse()  {
        Matrix result { *this };
        result.data = result.data.apply(
            [](ScalarType val){ return 1 / val; });
        return result;
    }

    /*
     * @brief Serialize to output stream
     *
     * @notes Defining templated friend functions inside a templated class saves
     *   on verbosity, see:
     *   - https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Making_New_Friends
     *   - https://stackoverflow.com/questions/4039817/
     *   - https://www.ibm.com/docs/en/zos/2.3.0?topic=only-friends-templates-c
     */
    friend std::ostream& operator<<(std::ostream& os,
                                    const Matrix<ScalarType, ColumnSize, RowSize>& m) {
        os << '{';
        for (uint32_t row_i { 0 }; row_i < m.rows; ++row_i) {
            os << '{';
            for (uint32_t col_i { 0 }; col_i < m.columns; ++col_i) {
                os << m(col_i, row_i);
                if (col_i < m.columns - 1)
                    os << ", ";
            }
            os << '}';
            if (row_i < m.rows - 1)
                os << ", ";
        }
        return os << '}';
    }

    // using templated class in a templated function, see:
    //   - https://stackoverflow.com/questions/10645085/
    template <typename InputScalarType>
    using return_CI_if_scalar_param = typename std::enable_if<
        std::is_scalar<InputScalarType>::value,
        typename Matrix<ScalarType, ColumnSize, RowSize>::CommaInitializer>::type;

    /*
     * @brief Input comma-delimited values into Matrix (see CommaInitializer)
     *
     * @notes - https://eigen.tuxfamily.org/dox/classEigen_1_1DenseBase.html#a0e575eb0ba6cc6bc5f347872abd8509d
     *   - https://stackoverflow.com/questions/17075506/operator-on-comma-separated-values-in-c
     */
    template <typename InputScalarType>
    friend auto operator<<(Matrix<ScalarType, ColumnSize, RowSize>& m,
                           InputScalarType val) -> return_CI_if_scalar_param<InputScalarType> {
        typename Matrix<
            ScalarType, ColumnSize, RowSize>::CommaInitializer comma_initializer(m, m.begin());
        return (comma_initializer, val);
    }
};


/*
 * @brief CommmaInitializer returns from Matrix::operator<<(scalar) to enable
 *   matrix value population with operator,
 *
 * @notes Adapted from OpenCV, which uses << and , overloads similar to Eigen, see:
 *   - https://github.com/opencv/opencv/blob/HEAD/modules/core/include/opencv2/core/mat.hpp#L523
 *   - https://github.com/opencv/opencv/blob/HEAD/modules/core/include/opencv2/core/mat.inl.hpp#L2972
 *   Defined outside Matrix definition as it uses Matrix in its own definition.
 */
template <typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix<ScalarType, ColumnSize, RowSize>::CommaInitializer {
public:
    using iterator_type = Matrix::iterator_type;

    // constructor called by `Matrix << v1`
    CommaInitializer(Matrix& mat, iterator_type iter) :
        matrix {mat}, it {iter} {}

    // enables Matrix population with comma-delimited values: `Matrix << v1, v2, ...`
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
