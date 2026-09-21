#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include "matrixlib/matrix.hpp"

using namespace mat;

namespace
{
	// Helper: build a matrix filled with a single value, for tests where the
	// exact numbers don't matter, only shape/behavior.
	Matrix MakeFilled(size_t rows, size_t cols, float value)
	{
		Matrix m(rows, cols);
		for (size_t r = 0; r < rows; ++r)
		{
			for (size_t c = 0; c < cols; ++c)
			{
				m(r, c) = value;
			}
		}
		return m;
	}

} // namespace

Matrix NaiveMultiplyReference(const Matrix& a, const Matrix& b)
{
	Matrix result(a.GetShape().rows, b.GetShape().cols);
	for (size_t r = 0; r < a.GetShape().rows; ++r)
	{
		for (size_t c = 0; c < b.GetShape().cols; ++c)
		{
			float sum = 0.0f;
			for (size_t k = 0; k < a.GetShape().cols; ++k)
			{
				sum += a(r, k) * b(k, c);
			}
			result(r, c) = sum;
		}
	}
	return result;
}

bool ApproxEqual(const Matrix& a, const Matrix& b, float epsilon = 1e-3f)
{
	if (a.GetShape() != b.GetShape())
	{
		return false;
	}
	const float* pa = a.begin();
	const float* pb = b.begin();
	for (size_t i = 0; i < a.Size(); ++i)
	{
		if (std::fabs(pa[i] - pb[i]) > epsilon)
		{
			return false;
		}
	}
	return true;
}

// Temporarily overrides Matrix::s_HardwareInfo for the lifetime of this
// guard, restoring the original value on destruction -- including if a
// REQUIRE fails and unwinds partway through.
// This only works safely because Catch2 runs test cases sequentially
// Will not work in parallel
class HardwareInfoOverrideGuard
{
public:
	explicit HardwareInfoOverrideGuard(mat::HardwareInfo overridden)
		: saved_(Matrix::s_HardwareInfo)
	{
		Matrix::s_HardwareInfo = overridden;
	}
	~HardwareInfoOverrideGuard()
	{
		Matrix::s_HardwareInfo = saved_;
	}
	HardwareInfoOverrideGuard(const HardwareInfoOverrideGuard&) = delete;
	HardwareInfoOverrideGuard& operator=(const HardwareInfoOverrideGuard&) = delete;

private:
	mat::HardwareInfo saved_;
};

// =======================================================================
// Basics
// =======================================================================

TEST_CASE("Matrix construction zero-initializes elements", "[matrix]")
{
	Matrix m(2, 3);
	REQUIRE(m.GetShape().rows == 2);
	REQUIRE(m.GetShape().cols == 3);
	for (std::size_t r = 0; r < m.GetShape().rows; r++)
	{
		for (std::size_t c = 0; c < m.GetShape().cols; c++)
		{
			REQUIRE(m(r, c) == 0.0);
		}
	}
}

TEST_CASE("Matrix elements can be read and written", "[matrix]")
{
	Matrix m(2, 2);
	m(0, 0) = 1.0;
	m(0, 1) = 2.0;
	m(1, 0) = 3.0;
	m(1, 1) = 4.0;

	REQUIRE(m(0, 0) == 1.0);
	REQUIRE(m(1, 1) == 4.0);
}


// =======================================================================
// Addition
// =======================================================================

TEST_CASE("Matrix addition sums elementwise", "[matrix][addition]")
{
	Matrix a{ {1.0f, 2.0f}, {3.0f, 4.0f} };
	Matrix b{ {5.0f, 6.0f}, {7.0f, 8.0f} };

	Matrix c = a + b;

	REQUIRE(c(0, 0) == 6.0f);
	REQUIRE(c(0, 1) == 8.0f);
	REQUIRE(c(1, 0) == 10.0f);
	REQUIRE(c(1, 1) == 12.0f);
}

TEST_CASE("Matrix addition preserves shape", "[matrix][addition]")
{
	Matrix a(3, 5);
	Matrix b(3, 5);
	Matrix c = a + b;

	REQUIRE(c.GetShape().rows == 3);
	REQUIRE(c.GetShape().cols == 5);
}

TEST_CASE("Matrix addition with zero is identity", "[matrix][addition]")
{
	Matrix a{ {1.0f, 2.0f}, {3.0f, 4.0f} };
	Matrix zero = MakeFilled(2, 2, 0.0f);

	Matrix c = a + zero;

	REQUIRE(c(0, 0) == a(0, 0));
	REQUIRE(c(0, 1) == a(0, 1));
	REQUIRE(c(1, 0) == a(1, 0));
	REQUIRE(c(1, 1) == a(1, 1));
}

TEST_CASE("Matrix addition throws on mismatched dimensions", "[matrix][addition]")
{
	Matrix a(2, 3);
	Matrix b(3, 2);
	REQUIRE_THROWS_AS(a + b, std::invalid_argument);
}


// =======================================================================
// Subtraction
// =======================================================================

TEST_CASE("Matrix subtraction computes elementwise difference", "[matrix][subtraction]")
{
	Matrix a{ {5.0f, 6.0f}, {7.0f, 8.0f} };
	Matrix b{ {1.0f, 2.0f}, {3.0f, 4.0f} };

	Matrix c = a - b;

	REQUIRE(c(0, 0) == 4.0f);
	REQUIRE(c(0, 1) == 4.0f);
	REQUIRE(c(1, 0) == 4.0f);
	REQUIRE(c(1, 1) == 4.0f);
}

TEST_CASE("Matrix minus itself is a zero matrix", "[matrix][subtraction]")
{
	Matrix a(4, 4);
	a.FillRandom();

	Matrix c = a - a;

	for (size_t i = 0; i < c.Size(); ++i)
	{
		REQUIRE(c.begin()[i] == 0.0f);
	}
}

TEST_CASE("Matrix subtraction with zero is identity", "[matrix][subtraction]")
{
	Matrix a{ {1.0f, 2.0f}, {3.0f, 4.0f} };
	Matrix zero = MakeFilled(2, 2, 0.0f);

	Matrix c = a - zero;

	REQUIRE(c(0, 0) == a(0, 0));
	REQUIRE(c(0, 1) == a(0, 1));
	REQUIRE(c(1, 0) == a(1, 0));
	REQUIRE(c(1, 1) == a(1, 1));
}

TEST_CASE("Matrix subtraction throws on mismatched dimensions", "[matrix][subtraction]")
{
	Matrix a(2, 3);
	Matrix b(3, 2);
	REQUIRE_THROWS_AS(a - b, std::invalid_argument);
}


// =======================================================================
// Unary negation
// =======================================================================

TEST_CASE("Unary negation negates every element", "[matrix][negation]")
{
	Matrix a{ {1.0f, -2.0f}, {0.0f, 4.0f} };

	Matrix c = -a;

	REQUIRE(c(0, 0) == -1.0f);
	REQUIRE(c(0, 1) == 2.0f);
	REQUIRE(c(1, 0) == 0.0f);
	REQUIRE(c(1, 1) == -4.0f);
}

TEST_CASE("Double negation returns the original matrix", "[matrix][negation]")
{
	Matrix a(6, 4);
	a.FillRandom();

	Matrix roundTripped = -(-a);

	for (size_t i = 0; i < a.Size(); ++i)
	{
		REQUIRE(roundTripped.begin()[i] == a.begin()[i]);
	}
}

TEST_CASE("Negation is consistent with subtraction from zero", "[matrix][negation][subtraction]")
{
	// Cross-checks negation and subtraction against each other rather than
	// hand-computed values: 0 - a should equal -a for any a.
	Matrix a(5, 5);
	a.FillRandom();
	Matrix zero(5, 5);

	Matrix viaSubtraction = zero - a;
	Matrix viaNegation = -a;

	for (size_t i = 0; i < a.Size(); ++i)
	{
		REQUIRE(viaSubtraction.begin()[i] == viaNegation.begin()[i]);
	}
}


// =======================================================================
// Compound assignment (+=, -=)
// =======================================================================

TEST_CASE("operator+= mutates in place and matches operator+", "[matrix][addition][compound]")
{
	Matrix a{ {1.0f, 2.0f}, {3.0f, 4.0f} };
	Matrix b{ {5.0f, 6.0f}, {7.0f, 8.0f} };
	Matrix expected = a + b; // computed before a is mutated below

	Matrix& ref = (a += b);

	REQUIRE(a(0, 0) == expected(0, 0));
	REQUIRE(a(0, 1) == expected(0, 1));
	REQUIRE(a(1, 0) == expected(1, 0));
	REQUIRE(a(1, 1) == expected(1, 1));
	REQUIRE(&ref == &a); // returns *this by reference, not a copy
}

TEST_CASE("operator-= mutates in place and matches operator-", "[matrix][subtraction][compound]")
{
	Matrix a{ {5.0f, 6.0f}, {7.0f, 8.0f} };
	Matrix b{ {1.0f, 2.0f}, {3.0f, 4.0f} };
	Matrix expected = a - b; // computed before a is mutated below

	Matrix& ref = (a -= b);

	REQUIRE(a(0, 0) == expected(0, 0));
	REQUIRE(a(0, 1) == expected(0, 1));
	REQUIRE(a(1, 0) == expected(1, 0));
	REQUIRE(a(1, 1) == expected(1, 1));
	REQUIRE(&ref == &a);
}

TEST_CASE("operator+= and operator-= throw on mismatched dimensions", "[matrix][compound]")
{
	Matrix a(2, 3);
	Matrix b(3, 2);

	REQUIRE_THROWS_AS(a += b, std::invalid_argument);
	REQUIRE_THROWS_AS(a -= b, std::invalid_argument);
}

TEST_CASE("Compound assignment chains left-to-right", "[matrix][compound]")
{
	// a += b -= c   parses as   a += (b -= c)
	Matrix a{ {10.0f, 10.0f} };
	Matrix b{ {5.0f, 5.0f} };
	Matrix c{ {2.0f, 2.0f} };

	a += (b -= c); // b becomes {3, 3}, then a becomes {13, 13}

	REQUIRE(b(0, 0) == 3.0f);
	REQUIRE(b(0, 1) == 3.0f);
	REQUIRE(a(0, 0) == 13.0f);
	REQUIRE(a(0, 1) == 13.0f);
}


// =======================================================================
// Multiplication
// =======================================================================

TEST_CASE("Matrix multiplication produces correct values (small, hand-checked)", "[matrix][multiplication]")
{
	// [1 2]   [5 6]   [1*5+2*7  1*6+2*8]   [19 22]
	// [3 4] * [7 8] = [3*5+4*7  3*6+4*8] = [43 50]
	Matrix a{ {1.0f, 2.0f}, {3.0f, 4.0f} };
	Matrix b{ {5.0f, 6.0f}, {7.0f, 8.0f} };

	Matrix c = a * b;

	REQUIRE(c.GetShape().rows == 2);
	REQUIRE(c.GetShape().cols == 2);
	REQUIRE(c(0, 0) == 19.0f);
	REQUIRE(c(0, 1) == 22.0f);
	REQUIRE(c(1, 0) == 43.0f);
	REQUIRE(c(1, 1) == 50.0f);
}

TEST_CASE("Matrix multiplication by identity is a no-op", "[matrix][multiplication]")
{
	Matrix a{ {1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f} };

	Matrix identity(3, 3);
	for (size_t i = 0; i < 3; ++i)
	{
		identity(i, i) = 1.0f;
	}

	Matrix c = a * identity;

	for (size_t r = 0; r < a.GetShape().rows; ++r)
	{
		for (size_t col = 0; col < a.GetShape().cols; ++col)
		{
			REQUIRE(c(r, col) == a(r, col));
		}
	}
}

TEST_CASE("Matrix multiplication by zero matrix gives zero matrix", "[matrix][multiplication]")
{
	Matrix a{ {1.0f, 2.0f}, {3.0f, 4.0f} };
	Matrix zero = MakeFilled(2, 2, 0.0f);

	Matrix c = a * zero;

	REQUIRE(c(0, 0) == 0.0f);
	REQUIRE(c(0, 1) == 0.0f);
	REQUIRE(c(1, 0) == 0.0f);
	REQUIRE(c(1, 1) == 0.0f);
}

TEST_CASE("Matrix multiplication produces correct shape for non-square operands", "[matrix][multiplication]")
{
	Matrix a(2, 3); // 2x3
	Matrix b(3, 4); // 3x4
	Matrix c = a * b;

	REQUIRE(c.GetShape().rows == 2);
	REQUIRE(c.GetShape().cols == 4);
}

TEST_CASE("Matrix multiplication throws on incompatible inner dimensions", "[matrix][multiplication]")
{
	Matrix a(2, 3);
	Matrix b(2, 3); // inner dims (3 vs 2) don't match
	REQUIRE_THROWS_AS(a * b, std::invalid_argument);
}

// --- Block-boundary edge cases ------------------------------------------

TEST_CASE("Matrix multiplication matches naive result for exact block-size matrices (16x16)", "[matrix][multiplication][blocking]")
{
	constexpr size_t n = Matrix::BLOCK_SIZE; // 16
	Matrix a(n, n);
	Matrix identity(n, n);

	for (size_t r = 0; r < n; ++r)
	{
		for (size_t c = 0; c < n; ++c)
		{
			a(r, c) = static_cast<float>(r + c);
		}
		identity(r, r) = 1.0f;
	}

	Matrix result = a * identity;

	for (size_t r = 0; r < n; ++r)
	{
		for (size_t c = 0; c < n; ++c)
		{
			REQUIRE(result(r, c) == a(r, c));
		}
	}
}

TEST_CASE("Matrix multiplication matches naive result for matrices smaller than one block (5x5)", "[matrix][multiplication][blocking]")
{
	Matrix a(5, 5);
	Matrix identity(5, 5);
	for (size_t i = 0; i < 5; ++i)
	{
		identity(i, i) = 1.0f;
	}
	for (size_t r = 0; r < 5; ++r)
	{
		for (size_t c = 0; c < 5; ++c)
		{
			a(r, c) = static_cast<float>(r * 5 + c);
		}
	}

	Matrix result = a * identity;

	for (size_t r = 0; r < 5; ++r)
	{
		for (size_t c = 0; c < 5; ++c)
		{
			REQUIRE(result(r, c) == a(r, c));
		}
	}
}

TEST_CASE("Matrix multiplication matches naive result across a block boundary (20x20)", "[matrix][multiplication][blocking]")
{
	// 20 = one full 16-block plus a 4-wide remainder -- exercises the
	// partial-block path that pure multiples of BLOCK_SIZE never touch.
	constexpr size_t n = 20;
	Matrix a(n, n);
	Matrix identity(n, n);
	for (size_t i = 0; i < n; ++i)
	{
		identity(i, i) = 1.0f;
	}
	for (size_t r = 0; r < n; ++r)
	{
		for (size_t c = 0; c < n; ++c)
		{
			a(r, c) = static_cast<float>((r * 31 + c * 7) % 13); // arbitrary but deterministic
		}
	}

	Matrix result = a * identity;

	for (size_t r = 0; r < n; ++r)
	{
		for (size_t c = 0; c < n; ++c)
		{
			REQUIRE(result(r, c) == a(r, c));
		}
	}
}

// --- AVX512, AVX2 and scalar individually -------------------------------

TEST_CASE("operator* falls back to AVX2 path when AVX512 is unavailable", "[matrix][multiplication][dispatch]")
{
	mat::HardwareInfo forced = Matrix::s_HardwareInfo;
	if (!forced.avx2)
	{
		SKIP("AVX2 not supported on this CPU -- nothing to fall back to");
	}
	forced.avx512f = false;
	HardwareInfoOverrideGuard guard(forced);

	Matrix a(20, 20);
	Matrix b(20, 20);
	a.FillRandom();
	b.FillRandom();

	Matrix expected = NaiveMultiplyReference(a, b);
	Matrix actual = a * b; // public operator*, should route to AVX2 now

	REQUIRE(ApproxEqual(actual, expected));
}

TEST_CASE("operator* falls back to scalar path when no SIMD is available", "[matrix][multiplication][dispatch]")
{
	mat::HardwareInfo forced = Matrix::s_HardwareInfo;
	forced.avx512f = false;
	forced.avx2 = false;
	HardwareInfoOverrideGuard guard(forced);

	Matrix a(20, 20);
	Matrix b(20, 20);
	a.FillRandom();
	b.FillRandom();

	Matrix expected = NaiveMultiplyReference(a, b);
	Matrix actual = a * b; // public operator*, should route to scalar Multiply now

	REQUIRE(ApproxEqual(actual, expected));
}

TEST_CASE("operator* uses AVX512 path when available", "[matrix][multiplication][dispatch]")
{
	if (!Matrix::s_HardwareInfo.avx512f)
	{
		SKIP("AVX512 not supported on this CPU");
	}

	Matrix a(20, 20);
	Matrix b(20, 20);
	a.FillRandom();
	b.FillRandom();

	Matrix expected = NaiveMultiplyReference(a, b);
	Matrix actual = a * b;

	REQUIRE(ApproxEqual(actual, expected));
}


// =======================================================================
// Transpose
// =======================================================================

TEST_CASE("Transpose swaps shape for non-square matrices", "[matrix][transpose]")
{
	Matrix a(3, 5);
	Matrix t = a.Transpose();

	REQUIRE(t.GetShape().rows == 5);
	REQUIRE(t.GetShape().cols == 3);
}

TEST_CASE("Transpose produces correct values (hand-checked small example)", "[matrix][transpose]")
{
	// [1 2 3]        [1 4]
	// [4 5 6]  -->   [2 5]
	//                [3 6]
	Matrix a{ {1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f} };
	Matrix t = a.Transpose();

	REQUIRE(t.GetShape().rows == 3);
	REQUIRE(t.GetShape().cols == 2);
	REQUIRE(t(0, 0) == 1.0f);
	REQUIRE(t(0, 1) == 4.0f);
	REQUIRE(t(1, 0) == 2.0f);
	REQUIRE(t(1, 1) == 5.0f);
	REQUIRE(t(2, 0) == 3.0f);
	REQUIRE(t(2, 1) == 6.0f);
}

TEST_CASE("Transpose is its own inverse (transposing twice returns the original)", "[matrix][transpose]")
{
	Matrix a(7, 11);
	a.FillRandom();

	Matrix roundTripped = a.Transpose().Transpose();

	REQUIRE(roundTripped.GetShape() == a.GetShape());
	for (size_t r = 0; r < a.GetShape().rows; ++r)
	{
		for (size_t c = 0; c < a.GetShape().cols; ++c)
		{
			REQUIRE(roundTripped(r, c) == a(r, c));
		}
	}
}

TEST_CASE("Transpose handles row and column vectors", "[matrix][transpose]")
{
	Matrix row(1, 6);
	row.FillRandom();
	Matrix t = row.Transpose();

	REQUIRE(t.GetShape().rows == 6);
	REQUIRE(t.GetShape().cols == 1);
	for (size_t i = 0; i < 6; ++i)
	{
		REQUIRE(t(i, 0) == row(0, i));
	}
}

TEST_CASE("Transpose is correct across block-boundary shapes", "[matrix][transpose][blocking]")
{
	const std::vector<std::pair<size_t, size_t>> shapes = {
		{16, 16},
		{5, 5},
		{20, 20},
		{17, 33}, // non-square, straddles a boundary in both dimensions
	};

	for (const auto& [rows, cols] : shapes)
	{
		Matrix a(rows, cols);
		a.FillRandom();

		Matrix t = a.Transpose();

		INFO("shape: " << rows << "x" << cols);
		REQUIRE(t.GetShape().rows == cols);
		REQUIRE(t.GetShape().cols == rows);

		for (size_t r = 0; r < rows; ++r)
		{
			for (size_t c = 0; c < cols; ++c)
			{
				REQUIRE(t(c, r) == a(r, c));
			}
		}
	}
}
