#pragma once
#include <iostream>

namespace mat
{
	// -------------------------
	// Shape struct
	// -------------------------
	struct Shape
	{
		size_t rows;
		size_t cols;

		bool operator==(const Shape& other) const;
		bool operator!=(const Shape& other) const;
	};


	// -------------------------
	// Non-member operators
	// -------------------------
	std::ostream& operator<<(std::ostream& os, const Shape& shape);
}
