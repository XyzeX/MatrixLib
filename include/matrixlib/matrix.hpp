#pragma once
#include <iostream>
#include <initializer_list>
#include <cstddef>
#include <memory>
#include <thread>

#include "shape.hpp"
#include "hardwareinfo.hpp"

namespace mat
{
	// -------------------------
	// Matrix class
	// -------------------------
	class Matrix
	{
	public:
		// -------------------------
		// Special member functions
		// -------------------------
		Matrix();
		explicit Matrix(const Shape& _shape);
		explicit Matrix(size_t rows, size_t cols);
		Matrix(std::initializer_list<float> init);
		Matrix(std::initializer_list<std::initializer_list<float>> init);
		~Matrix();

		Matrix(const Matrix& other);							// copy constructor
		Matrix& operator=(const Matrix& other);					// copy assignment
		Matrix(Matrix&& other) noexcept = default;				// move constructor
		Matrix& operator=(Matrix&& other) noexcept = default;	// move assignment

		// -------------------------
		// Member methods
		// -------------------------
		void FillRandom();
		[[nodiscard]] Matrix Reshape(size_t newRows, size_t newCols) const;
		[[nodiscard]] Matrix Transpose() const;
		[[nodiscard]] const Shape& GetShape() const { return shape; }
		[[nodiscard]] size_t Size() const { return size; }

		// Iterator
		float* begin() { return arr.get(); }
		float* end() { return arr.get() + size; }

		const float* begin() const { return arr.get(); }
		const float* end() const { return arr.get() + size; }

		template <typename Func>
		Matrix Apply(Func f) const
		{
			Matrix result(shape);
			for (size_t c = 0; c < size; c++)
			{
				result.arr[c] = f(arr[c]);
			}
			return result;
		}

		Matrix Sigmoid() const;
		Matrix Log() const;

		float Sum() const;
		Matrix SumAxis(int axis) const; // 0 = sum down each column, 1 = sum across each row

		Matrix EntryWiseMultiply(const Matrix& other) const;

		// -------------------------
		// Operators
		// -------------------------
		float& operator()(size_t r, size_t c) { return arr[ArrIndex(r, c)]; }
		const float& operator()(size_t r, size_t c) const { return arr[ArrIndex(r, c)]; }

		Matrix& operator+=(const Matrix& other);
		Matrix& operator-=(const Matrix& other);

		[[nodiscard]] Matrix operator-() const;
		[[nodiscard]] Matrix operator+(const Matrix& other) const;
		[[nodiscard]] Matrix operator-(const Matrix& other) const;
		[[nodiscard]] Matrix operator*(const Matrix& other) const;

		[[nodiscard]] Matrix operator+(float scalar) const;
		[[nodiscard]] Matrix operator-(float scalar) const;
		[[nodiscard]] Matrix operator*(float scalar) const;

	private:
		// -------------------------
		// Helpers
		// -------------------------
		size_t ArrIndex(size_t r, size_t c) const;
		Matrix Multiply(const Matrix& other) const;
		Matrix MultiplyAVX2(const Matrix& other) const;
		Matrix MultiplyAVX512f(const Matrix& other) const;

	private:
		Shape shape;
		size_t size;
		std::unique_ptr<float[]> arr;

	public:
		inline static constexpr size_t BLOCK_SIZE = 16; // At least 16 for AVX512 with float
		inline static HardwareInfo s_HardwareInfo = HardwareInfo::Detect();
		static constexpr size_t Infer = static_cast<size_t>(-1);
	};


	// -------------------------
	// Non-member operators
	// -------------------------
	std::ostream& operator<<(std::ostream& os, const Matrix& m);

	Matrix operator+(float scalar, const Matrix& m);
	Matrix operator-(float scalar, const Matrix& m);
	Matrix operator*(float scalar, const Matrix& m);
}
