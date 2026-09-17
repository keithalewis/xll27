// ref.h - XLREF/XLREF12
#pragma once
#include "xltraits.h"

namespace xll {

	// XLREF/XLREF12
	template<convertible_to_ref_t X>
	constexpr size_t rows(const X& ref)
	{
		return ref.rwLast - ref.rwFirst + 1;
	}
	template<convertible_to_ref_t X>
	constexpr size_t columns(const X& ref)
	{
		return ref.colLast - ref.colFirst + 1;
	}
	template<convertible_to_ref_t X>
	constexpr size_t size(const X& ref)
	{
		return rows(ref) * columns(ref);
	}

	template<convertible_to_ref_t X>
	constexpr bool equal(const X& lhs, const X& rhs)
	{
		return  lhs.rwFirst == rhs.rwFirst
			and lhs.rwLast == rhs.rwLast
			and lhs.colFirst == rhs.colFirst
			and lhs.colLast == rhs.colLast;
	}

	// XLREF/XLREF12
	template<is_ref_t R>
	struct RefX : public R {
		using rw = ref_traits<R>::rw_t;
		using col = ref_traits<R>::col_t;

		constexpr RefX(const R& ref)
			: R(ref)
		{}
		// Upper left corner (x, y) having width w and height h.
		constexpr RefX(rw x, col y, rw w, col h)
			: R({ x, x + w - 1, y, y + h - 1 })
		{}
		constexpr bool operator==(const RefX& rhs) const
		{
			return equal(*this, rhs);
		}
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
#ifdef _DEBUG
	static_assert(rows(Ref(1, 2, 3, 4)) == 3);
	static_assert(columns(Ref(1, 2, 3, 4)) == 4);
	static_assert(size(Ref(1, 2, 3, 4)) == 3 * 4);
	static_assert(size(Ref(XLREF12{ 1, 4, 2, 6 })) == 20);
	static_assert(Ref(1, 2, 3, 4) == Ref(1, 2, 3, 4));
	static_assert(Ref(1, 2, 3, 4) == XLREF12{ 1, 3, 2, 5 });
#endif // _DEBUG

} // namespace xll