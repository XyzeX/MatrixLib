#include <catch2/catch_test_macros.hpp>
#include "matrixlib/matrix.hpp"

using matrixlib::Matrix;

TEST_CASE("Matrix construction zero-initializes elements", "[matrix]") {
    Matrix m(2, 3);
    REQUIRE(m.rows() == 2);
    REQUIRE(m.cols() == 3);
    for (std::size_t r = 0; r < m.rows(); ++r) {
        for (std::size_t c = 0; c < m.cols(); ++c) {
            REQUIRE(m(r, c) == 0.0);
        }
    }
}

TEST_CASE("Matrix elements can be read and written", "[matrix]") {
    Matrix m(2, 2);
    m(0, 0) = 1.0;
    m(0, 1) = 2.0;
    m(1, 0) = 3.0;
    m(1, 1) = 4.0;

    REQUIRE(m(0, 0) == 1.0);
    REQUIRE(m(1, 1) == 4.0);
}

TEST_CASE("Matrix addition sums elementwise", "[matrix]") {
    Matrix a(2, 2);
    Matrix b(2, 2);
    a(0, 0) = 1.0; a(0, 1) = 2.0; a(1, 0) = 3.0; a(1, 1) = 4.0;
    b(0, 0) = 5.0; b(0, 1) = 6.0; b(1, 0) = 7.0; b(1, 1) = 8.0;

    Matrix c = a + b;

    REQUIRE(c(0, 0) == 6.0);
    REQUIRE(c(0, 1) == 8.0);
    REQUIRE(c(1, 0) == 10.0);
    REQUIRE(c(1, 1) == 12.0);
}

TEST_CASE("Matrix addition throws on mismatched dimensions", "[matrix]") {
    Matrix a(2, 2);
    Matrix b(3, 3);
    REQUIRE_THROWS_AS(a + b, std::invalid_argument);
}
