#ifndef MATRIX_HH
#define MATRIX_HH

#include <array>

#include <cstdint>

#include <iterator>   // forward_iterator_tag
#include <cstddef>    // ptrdiff_t


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




// https://eigen.tuxfamily.org/dox/classEigen_1_1Matrix.html
// template<typename Scalar_, int Rows_, int Cols_, int Options_, int MaxRows_, int MaxCols_>
// class Eigen::Matrix< Scalar_, Rows_, Cols_, Options_, MaxRows_, MaxCols_ >

template<typename ScalarType, uint32_t ColumnSize, uint32_t RowSize>
class Matrix {
private:
    std::array<std::array<ScalarType, RowSize>, ColumnSize> matrix;

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

    friend std::ostream& operator<<(std::ostream& os, const Matrix& m);

    struct Iterator {
        friend class Matrix;  // access to matrix

        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ScalarType;
        using pointer           = ScalarType*;
        using reference         = ScalarType&;

        Iterator(pointer ptr, Matrix& mat) : m_ptr(ptr), m(mat) {}

        reference operator*() const { return *m_ptr; }
        pointer operator->() { return m_ptr; }
        // prefix ++
        Iterator& operator++() {
            if (m_ptr + 1 == m.matrix[row_i].end() &&
                row_i < m.row_ct - 1) {
                ++row_i;
                m_ptr = m.matrix[row_i].begin();
            } else {
                m_ptr++;
            }
            return *this;
        }
        // dummy int param indicates postfix ++, see:
        //   - https://stackoverflow.com/q/3574831
        Iterator operator++(int) { Iterator tmp { *this }; ++(*this); return tmp; }
        friend bool operator== (const Iterator& a, const Iterator& b) {
            return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) {
            return a.m_ptr != b.m_ptr; };

    private:
        pointer m_ptr;
        uint32_t row_i { 0 };
        Matrix& m;
    };

    Iterator begin() { return Iterator(matrix[0].begin(), *this); }
    Iterator end()   { return Iterator(matrix[row_ct - 1].end(), *this); }

    template <typename ScalarParamType>
    Matrix operator*(const ScalarParamType& s);

    // simplify from matrix * matrix to matrix * vector due to size of return matrix being dependent on order of largest
    template <typename MatrixType>
    Matrix operator*(const MatrixType& m);

    // like []
    template <typename AccessIndexType>
    scalar_type operator()(AccessIndex &s);

    //   finds the minimum of all coefficients of *this and puts in *index its location
    template <typename MinCoeffIndexType>
    scalar_type minCoeff(MinCoeffIndexType *index);

    //   std::abs(a[i]) for a in matrix
    void cwiseAbs();

    //   a[i] = 1 / a[i] for a in matrix
    void cwiseInverse();
};

std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    // TBD: change after building iterators?
    os << '{';
    for (auto row_it {m.matrix.begin()}; row_it != m.matrix.end(); ++row_it) {
        os << '{';
        for (auto col_it {row_it->begin()}; col_it != row_it->end(); ++col_it) {
            os << *col_it;
            if (col_it + 1 != row_it->end())
                os << ", ";
        }
        os << '}';
        if (row_it + 1 != m.matrix.end())
                os << ", ";
    }
    return os << '}';
}

// Vector* types are _column_ vectors
using Vector2i = Matrix<int, 2, 1>;
using Vector2d = Matrix<double, 2, 1>;

using Matrix2d = Matrix<double, 2, 2>;

#endif  // MATRIX_HH
