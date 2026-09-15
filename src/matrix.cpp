#include "matrixlib/matrix.hpp"

namespace matrixlib {

Matrix::Matrix(std::size_t rows, std::size_t cols)
    : rows_(rows), cols_(cols), data_(rows * cols, 0.0) {}

double& Matrix::operator()(std::size_t r, std::size_t c) {
    if (r >= rows_ || c >= cols_) {
        throw std::out_of_range("Matrix index out of range");
    }
    return data_[r * cols_ + c];
}

double Matrix::operator()(std::size_t r, std::size_t c) const {
    if (r >= rows_ || c >= cols_) {
        throw std::out_of_range("Matrix index out of range");
    }
    return data_[r * cols_ + c];
}

Matrix Matrix::operator+(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must match");
    }
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + other.data_[i];
    }
    return result;
}

} // namespace matrixlib
