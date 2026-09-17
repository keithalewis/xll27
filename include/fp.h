#pragma once

#include <new>
#include <cstddef>

namespace xll {
	struct FP {
		int rows;
		int columns;
		double array[1]; // struct hack: real storage is rows*columns doubles
	};
}