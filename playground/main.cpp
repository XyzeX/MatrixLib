#include "matrixlib/matrix.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
	mat::Matrix a(2, 2);
	a(0, 0) = 1.0; a(0, 1) = 2.0;
	a(1, 0) = 3.0; a(1, 1) = 4.0;

	mat::Matrix b(2, 2);
	b(0, 0) = 5.0; b(0, 1) = 6.0;
	b(1, 0) = 7.0; b(1, 1) = 8.0;

	mat::Matrix c = a + b;

	for (std::size_t r = 0; r < c.GetShape().rows; r++)
	{
		for (std::size_t col = 0; col < c.GetShape().cols; col++)
		{
			std::cout << c(r, col) << " ";
		}
		std::cout << "\n";
	}
}
