// ref.h - XLREF/XLREF12
#pragma once
#include "xltraits.h"

namespace xll {

	// XLREF/XLREF12
	template<is_ref_t X>
	constexpr size_t rows(const X& ref)
	{
		return ref.rwLast - ref.rwFirst + 1;
	}
	template<is_ref_t X>
	constexpr size_t columns(const X& ref)
	{
		return ref.colLast - ref.colFirst + 1;
	}
	template<is_ref_t X>
	constexpr size_t size(const X& ref)
	{
		return rows(ref) * columns(ref);
	}
	template<is_ref_t X>
	constexpr bool equal(const X& lhs, const X& rhs)
	{
		return lhs.rwFirst == rhs.rwFirst
			and lhs.rwLast == rhs.rwLast
			and lhs.colFirst == rhs.colFirst
			and lhs.colLast == rhs.colLast;
	}
	// XLREF/XLREF12
	template<is_ref_t X>
	struct RefX : public X {
		using rw = traits<X>::ref_rw_t;
		using col = traits<X>::ref_col_t;

		constexpr RefX(const traits<X>::ref_t& ref)
			: traits<X>::ref_t(ref)
		{}
		// Upper left corner (x, y) having width w and height h.
		constexpr RefX(rw x, col y, rw w, col h)
			: X({ x, x + w - 1, y + h - 1 })
		{}
		constexpr bool operator==(const RefX& rhs) const
		{
			return equal(*this, rhs);
		}
		operator
			RefX& move(rw dx, col dy)
		{
			this->rwFirst += dx;
			this->rwLast += dx;
			this->colFirst += dy;
			this->colLast += dy;

			return *this;
		}
	};
	using Ref4 = RefX<XLREF>;
	using Ref = RefX<XLREF12>;

} // namespace xll