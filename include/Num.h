// Num.h - xltypeNum
#pragma once
#include <limits>
#include "xloper.h"

namespace xll {

	template<class X>
	struct NumX : public X {
		// Default to NaN so that uninitialized XNum is not a valid number.
		constexpr NumX(double num = std::numeric_limits<double>::quiet_NaN()) 
			: X{ X::val.num = num, X::xltype = xltypeNum }	
		{ }
		constexpr operator double() const { return X::val.num; }
	};

	using Num4 = NumX<XLOPER>;
	using Num = NumX<XLOPER12>;

#ifdef _DEBUG
	static_assert(Num() != Num(), "Default XNum should be NaN");
	static_assert(Num(1.23) == 1.23, "Num conversion failed");
	static_assert(Num4(1.23) == 1.23, "Num4 conversion failed");
#endif // _DEUBG

} // namespace xll