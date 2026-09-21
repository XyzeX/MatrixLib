#include "matrixlib/shape.hpp"

namespace mat
{
	// -------------------------
	// Shape struct
	// -------------------------

	bool Shape::operator==(const Shape& other) const
	{
		return rows == other.rows && cols == other.cols;
	}

	bool Shape::operator!=(const Shape& other) const
	{
		return !(*this == other);
	}


	// -------------------------
	// Non-member operators
	// -------------------------
	std::ostream& operator<<(std::ostream& os, const Shape& shape)
	{
		return os << '(' << shape.rows << ", " << shape.cols << ')';
	}
}
