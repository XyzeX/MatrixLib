#include "matrixlib/matrix.hpp"

#include <cstdlib>
#include <ctime>
#include <vector>
#include <random>

namespace mat
{
	typedef unsigned long long ull;

	// One generator per thread, seeded once (not per call)
	inline std::mt19937& RandomEngine()
	{
		thread_local std::mt19937 generator{ std::random_device{}() };
		return generator;
	}

	// -------------------------
	// Matrix class
	// Special member functions
	// -------------------------
	Matrix::Matrix()
		: shape({ 0, 0 }), size(0)
	{}

	Matrix::Matrix(const Shape& _shape)
		: shape(_shape), size(shape.rows* shape.cols), arr(std::make_unique<float[]>(size))
	{}

	Matrix::Matrix(size_t rows, size_t cols)
		: shape({ rows, cols }), size(rows* cols), arr(std::make_unique<float[]>(size))
	{}

	Matrix::Matrix(std::initializer_list<float> init)
		: shape({ 1, init.size() }), size(shape.cols), arr(std::make_unique<float[]>(size))
	{
		std::copy_n(init.begin(), init.size(), begin());
	}

	Matrix::Matrix(std::initializer_list<std::initializer_list<float>> init)
		: shape({ init.size(), init.begin()->size() }), size(shape.rows* shape.cols), arr(std::make_unique<float[]>(size))
	{
		size_t r = 0;
		for (const auto& row : init)
		{
			if (row.size() != shape.cols)
			{
				throw std::runtime_error("Matrix must be rectangular!");
			}

			std::copy_n(row.begin(), shape.cols, begin() + r * shape.cols);
			r++;
		}
	}

	Matrix::~Matrix() {}

	// copy constructor
	Matrix::Matrix(const Matrix& other)
		: shape(other.shape), size(other.size), arr(std::make_unique<float[]>(size))
	{
		std::copy_n(other.begin(), size, begin());
	}

	// copy assignment
	Matrix& Matrix::operator=(const Matrix& other)
	{
		if (this == &other)
		{
			return *this;
		}

		shape = other.shape;
		size = other.size;
		arr = std::make_unique<float[]>(size);

		std::copy_n(other.begin(), size, begin());
		return *this;
	}


	// -------------------------
	// Member methods
	// -------------------------
	void Matrix::FillRandom()
	{
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		std::mt19937& rng = RandomEngine();
		for (size_t r = 0; r < shape.rows; r++)
		{
			for (size_t c = 0; c < shape.cols; c++)
			{
				(*this)(r, c) = dist(rng);
			}
		}
	}

	Matrix Matrix::Reshape(size_t newX, size_t newY) const
	{
		// Look at numpy reshape
		throw std::runtime_error("RESHAPE NOT DEFINED!\n");
	}

	Matrix Matrix::Transpose() const
	{
		Matrix m = Matrix(shape);

		for (int r = 0; r < shape.rows; r++)
		{
			for (int c = 0; c < shape.cols; c++)
			{
				m(c, r) = (*this)(r, c);
			}
		}
		return m;
	}


	// -------------------------
	// Operators
	// -------------------------
	Matrix& Matrix::operator+=(const Matrix& other)
	{
		if (shape != other.shape)
		{
			throw std::invalid_argument("Matrix dimensions must match completely!");
		}

		for (size_t c = 0; c < size; c++)
		{
			arr[c] += other.arr[c];
		}

		return *this;
	}

	Matrix& Matrix::operator-=(const Matrix& other)
	{
		if (shape != other.shape)
		{
			throw std::invalid_argument("Matrix dimensions must match completely!");
		}

		for (size_t c = 0; c < size; c++)
		{
			arr[c] -= other.arr[c];
		}

		return *this;
	}

	Matrix Matrix::operator-() const
	{
		Matrix m(shape.rows, shape.cols);

		for (size_t c = 0; c < size; c++)
		{
			m.arr[c] = -arr[c];
		}
		return m;
	}

	Matrix Matrix::operator+(const Matrix& other) const
	{
		if (shape != other.shape)
		{
			throw std::invalid_argument("Matrix dimensions must match completely!");
		}

		Matrix m(shape.rows, shape.cols);

		for (size_t c = 0; c < size; c++)
		{
			m.arr[c] = arr[c] + other.arr[c];
		}
		return m;
	}

	Matrix Matrix::operator-(const Matrix& other) const
	{
		if (shape != other.shape)
		{
			throw std::invalid_argument("Matrix dimensions must match completely!");
		}

		Matrix m(shape.rows, shape.cols);

		for (size_t c = 0; c < size; c++)
		{
			m.arr[c] = arr[c] - other.arr[c];
		}
		return m;
	}

	Matrix Matrix::operator*(const Matrix& other) const
	{
		if (shape.cols != other.shape.rows)
		{
			throw std::invalid_argument("Matrix dimensions must match multiplication rules!");
		}

		if (!s_HardwareInfo.avx2
			|| size < BLOCK_SIZE * BLOCK_SIZE
			|| shape.rows < BLOCK_SIZE
			|| shape.cols < BLOCK_SIZE
			)
		{
			return Multiply(other);
		}

		if (s_HardwareInfo.avx512f)
		{
			return MultiplyAVX512f(other);
		}

		return MultiplyAVX2(other);
	}


	// -------------------------
	// Helpers
	// -------------------------
	size_t Matrix::ArrIndex(size_t r, size_t c) const
	{
		return r * shape.cols + c;
	}

	Matrix Matrix::Multiply(const Matrix& other) const
	{
		Matrix m = Matrix(shape.rows, other.shape.cols);
		std::fill_n(m.begin(), m.size, 0.0f);

		for (size_t r = 0; r < m.shape.rows; r++)
		{
			for (size_t i = 0; i < shape.cols; i++)
			{
				for (size_t c = 0; c < m.shape.cols; c++)
				{
					m(r, c) += (*this)(r, i) * other(i, c);
				}
			}
		}
		return m;
	}

	Matrix Matrix::MultiplyAVX2(const Matrix& other) const
	{
		Matrix m = Matrix(shape.rows, other.shape.cols);
		std::fill_n(m.begin(), m.size, 0.0f);

		alignas(64) float localA[BLOCK_SIZE][BLOCK_SIZE];
		alignas(64) float localB[BLOCK_SIZE][BLOCK_SIZE];
		alignas(64) float localC[BLOCK_SIZE][BLOCK_SIZE];
		const size_t blockNumRows = shape.rows / BLOCK_SIZE;
		const size_t blockNumCols = shape.cols / BLOCK_SIZE;

		auto threads = std::make_unique<std::thread[]>(s_HardwareInfo.threadCount);

		for (size_t br = 0; br < blockNumRows; br++)
		{
			for (size_t bc = 0; bc < blockNumCols; bc++)
			{
				// Clear localC
				memset(localC, 0, sizeof(localC));

				// Iterate through all blocks at this localC
				for (size_t bi = 0; bi < blockNumRows; bi++)
				{
					// Copy to local block
					for (size_t r = 0; r < BLOCK_SIZE; r++)
					{
						for (size_t c = 0; c < BLOCK_SIZE; c++)
						{
							localA[r][c] = (*this)(br * BLOCK_SIZE + r, bi * BLOCK_SIZE + c);
							localB[r][c] = other(bi * BLOCK_SIZE + r, bc * BLOCK_SIZE + c);
						}
					}

					// Multiplication using AVX/AVX2 SIMD instructions
					for (size_t r = 0; r < BLOCK_SIZE; r++)
					{
						for (size_t c = 0; c < BLOCK_SIZE; c += 8)
						{
							__m256 sum = _mm256_load_ps(&localC[r][c]);
							for (size_t i = 0; i < BLOCK_SIZE; i++)
							{
								const __m256 a = _mm256_set1_ps(localA[r][i]);
								const __m256 b = _mm256_load_ps(&localB[i][c]);
								sum = _mm256_fmadd_ps(a, b, sum);
							}
							_mm256_store_ps(&localC[r][c], sum);
						}
					}
				}

				// Save result from localC
				for (size_t r = 0; r < BLOCK_SIZE; r++)
				{
					for (size_t c = 0; c < BLOCK_SIZE; c++)
					{
						m(br * BLOCK_SIZE + r, bc * BLOCK_SIZE + c) += localC[r][c];
					}
				}
			}
		}
		return m;
	}

	Matrix Matrix::MultiplyAVX512f(const Matrix& other) const
	{
		Matrix m = Matrix(shape.rows, other.shape.cols);
		std::fill_n(m.begin(), m.size, 0.0f);

		const size_t blockNumRows = shape.rows / BLOCK_SIZE;
		const size_t blockNumCols = shape.cols / BLOCK_SIZE;
		const size_t threadNumRows = blockNumRows / s_HardwareInfo.threadCount;

		auto threads = std::make_unique<std::thread[]>(s_HardwareInfo.threadCount);

		for (size_t thread_i = 0; thread_i < s_HardwareInfo.threadCount; thread_i++)
		{
			const size_t brStart = thread_i * threadNumRows;
			size_t brEnd = brStart + threadNumRows;
			if (thread_i == s_HardwareInfo.threadCount - 1)
			{
				brEnd = blockNumRows;
			}

			threads[thread_i] = std::thread(
				[brStart, brEnd, blockNumCols, blockNumRows,
				this, &m, &other]()
			{
				alignas(64) float localA[BLOCK_SIZE][BLOCK_SIZE];
				alignas(64) float localB[BLOCK_SIZE][BLOCK_SIZE];
				alignas(64) float localC[BLOCK_SIZE][BLOCK_SIZE];
				for (size_t br = brStart; br < brEnd; br++)
				{
					for (size_t bc = 0; bc < blockNumCols; bc++)
					{
						// Clear localC
						memset(localC, 0, sizeof(localC));

						// Iterate through all blocks at this localC
						for (size_t bi = 0; bi < blockNumRows; bi++)
						{
							// Copy to local block
							for (size_t r = 0; r < BLOCK_SIZE; r++)
							{
								for (size_t c = 0; c < BLOCK_SIZE; c++)
								{
									localA[r][c] = (*this)(br * BLOCK_SIZE + r, bi * BLOCK_SIZE + c);
									localB[r][c] = other(bi * BLOCK_SIZE + r, bc * BLOCK_SIZE + c);
								}
							}

							// Multiplication using AVX512 SIMD instructions
							for (size_t r = 0; r < BLOCK_SIZE; r++)
							{
								for (size_t c = 0; c < BLOCK_SIZE; c += 16)
								{
									__m512 sum = _mm512_load_ps(&localC[r][c]);
									for (size_t i = 0; i < BLOCK_SIZE; i++)
									{
										const __m512 a = _mm512_set1_ps(localA[r][i]);
										const __m512 b = _mm512_load_ps(&localB[i][c]);
										sum = _mm512_fmadd_ps(a, b, sum);
									}
									_mm512_store_ps(&localC[r][c], sum);
								}
							}
						}

						// Save result from localC
						for (size_t r = 0; r < BLOCK_SIZE; r++)
						{
							for (size_t c = 0; c < BLOCK_SIZE; c++)
							{
								m(br * BLOCK_SIZE + r, bc * BLOCK_SIZE + c) += localC[r][c];
							}
						}
					}
				}
			});
		}

		const size_t remainingRowsStart = blockNumRows * BLOCK_SIZE;
		const size_t remainingColsStart = blockNumCols * BLOCK_SIZE;

		// Calculate remainder not in block
		for (size_t r = remainingRowsStart; r < shape.rows; r++)
		{
			for (size_t i = 0; i < shape.cols; i++)
			{
				for (size_t c = 0; c < shape.cols; c++)
				{
					m(r, c) += (*this)(r, i) * other(i, c);
				}
			}
		}

		for (size_t r = 0; r < remainingRowsStart; r++)
		{
			for (size_t i = 0; i < shape.cols; i++)
			{
				for (size_t c = remainingColsStart; c < shape.cols; c++)
				{
					m(r, c) += (*this)(r, i) * other(i, c);
				}
			}
		}

		for (size_t thread_i = 0; thread_i < s_HardwareInfo.threadCount; thread_i++)
		{
			threads[thread_i].join();
		}

		// Calculate remainders inside blocks
		for (size_t r = 0; r < remainingRowsStart; r++)
		{
			for (size_t i = remainingColsStart; i < shape.cols; i++)
			{
				for (size_t c = 0; c < remainingColsStart; c++)
				{
					m(r, c) += (*this)(r, i) * other(i, c);
				}
			}
		}
		return m;
	}


	// -------------------------
	// Non-member operators
	// -------------------------
	std::ostream& operator<<(std::ostream& os, const Matrix& m)
	{
		const Shape& shape = m.GetShape();
		for (size_t r = 0; r < shape.rows; r++)
		{
			os << '[';
			for (size_t c = 0; c < shape.cols; c++)
			{
				if (c != 0)
					os << ", ";
				os << m(r, c);
			}
			os << "]\n";
		}
		return os;
	}
}
