#include "matrixlib/matrix.hpp"

#include <iostream>
#include <cmath>
#include <chrono>

// https://blog.stackademic.com/learn-to-build-a-neural-network-from-scratch-yes-really-cac4ca457efc
const size_t nLayers = 3;
const int n[nLayers + 1] = { 2, 3, 3, 1 };

double RunTest(const mat::Shape& shape)
{
	std::cout << "------------------------------------------------\n";
	if (mat::Matrix::s_HardwareInfo.avx512f)
		std::cout << "AVX512\n";
	else if (mat::Matrix::s_HardwareInfo.avx2)
		std::cout << "AVX2\n";
	std::cout << "TC: " << mat::Matrix::s_HardwareInfo.threadCount << "\n";

	mat::Matrix A(shape.rows, shape.cols);
	mat::Matrix B(shape.cols, shape.rows);
	A.FillRandom();
	B.FillRandom();

	const auto start = std::chrono::high_resolution_clock::now();
	const mat::Matrix C = A * B;
	const auto end = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<double, std::milli> duration_ms = end - start;
	return duration_ms.count();
}

void Benchmark()
{
	double time;
	const mat::Shape shape{ 2048, 2048 };

	time = RunTest(shape);
	std::cout << "Multiplication took " << time << " ms\n";

	mat::Matrix::s_HardwareInfo.threadCount = 1;
	time = RunTest(shape);
	std::cout << "Multiplication took " << time << " ms\n";

	mat::Matrix::s_HardwareInfo.threadCount = 16;
	mat::Matrix::s_HardwareInfo.avx512f = false;
	time = RunTest(shape);
	std::cout << "Multiplication took " << time << " ms\n";

	mat::Matrix::s_HardwareInfo.threadCount = 1;
	time = RunTest(shape);
	std::cout << "Multiplication took " << time << " ms\n";

	mat::Matrix::s_HardwareInfo.avx2 = false;
	time = RunTest(shape);
	std::cout << "Multiplication took " << time << " ms\n";
}

float BinaryCrossEntropy(const mat::Matrix& y, const mat::Matrix& y_hat)
{
	mat::Matrix losses = -(
		y.EntryWiseMultiply(y_hat.Log())
		+ (1.0f - y).EntryWiseMultiply((1.0f - y_hat).Log())
	);

	const float m = static_cast<float>(y_hat.Size());
	mat::Matrix summed_losses = (1.0f / m) * losses.SumAxis(1);

	return summed_losses.Sum();
}

int main(int argc, char* argv[])
{
	//Benchmark();

	mat::Matrix W[nLayers];	// Weights ??
	mat::Matrix b[nLayers];	// bias
	mat::Matrix X;			// Training data
	mat::Matrix Y;			// Training answers

	for (size_t c = 0; c < nLayers; c++)
	{
		W[c] = mat::Matrix(n[c + 1], n[c]);
		b[c] = mat::Matrix(n[c + 1], 10);

		W[c].FillRandom();
		b[c].FillRandom();
	}


	// prepare_data()
	X = mat::Matrix({
		{ 150, 70 },
		{ 254, 73 },
		{ 312, 68 },
		{ 120, 60 },
		{ 154, 61 },
		{ 212, 65 },
		{ 216, 67 },
		{ 145, 67 },
		{ 184, 64 },
		{ 130, 69 }
	});
	mat::Matrix A0 = X.Transpose();

	mat::Matrix y({ 0, 1, 1, 0, 0, 1, 1, 0, 1, 0 });
	Y = y.Reshape(n[3], mat::Matrix::Infer);

	// Layer 1 calculations
	mat::Matrix Z1 = (W[0] * A0) + b[0];
	mat::Matrix A1 = Z1.Sigmoid();

	// Layer 2 calculations
	mat::Matrix Z2 = (W[1] * A1) + b[1];
	mat::Matrix A2 = Z2.Sigmoid();

	// Layer 3 calculations
	mat::Matrix Z3 = W[2] * A2 + b[2];
	mat::Matrix y_hat = Z3.Sigmoid();

	std::cout << "Cost: " << BinaryCrossEntropy(y, y_hat) << "\n";
}
