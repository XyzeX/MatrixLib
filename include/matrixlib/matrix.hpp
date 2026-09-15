#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace matrixlib {

class Matrix {
public:
    Matrix(std::size_t rows, std::size_t cols);

    [[nodiscard]] std::size_t rows() const noexcept { return rows_; }
    [[nodiscard]] std::size_t cols() const noexcept { return cols_; }

    double& operator()(std::size_t r, std::size_t c);
    double operator()(std::size_t r, std::size_t c) const;

    Matrix operator+(const Matrix& other) const;

private:
    std::size_t rows_;
    std::size_t cols_;
    std::vector<double> data_;
};

} // namespace matrixlib
