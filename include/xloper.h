// xloper.h - XLOPER/XLOPER12 traits
#pragma once
#define WINDOWS_LEAN_AND_MEAN                   
#include <Windows.h>
#include "XLCALL.H"
#include <algorithm>
#include <compare>
#include <concepts>

namespace xll {

	template<class X>
	concept is_char_t = std::same_as<X, CHAR> || std::same_as<X, WCHAR>;
	template<class X>
	concept is_xloper_t = std::same_as<X, XLOPER> || std::same_as<X, XLOPER12>;
	template<class X>
	concept is_ref_t = std::same_as<X, XLREF> || std::same_as<X, XLREF12>;

	template<is_xloper_t X>
	struct traits { };

	template<>
	struct traits<XLOPER> {
		using type_t = WORD; // val.type
		using char_t = CHAR;
		using int_t = short int;
		using err_t = WORD;
		using rw_t = unsigned short int;
		using col_t = unsigned short int;
		using ref_rw_t = WORD;
		using ref_col_t = BYTE;
		using ref_t = XLREF;
		using idsheet_t = IDSHEET;
	};
	template<>
	struct traits<XLOPER12> {
		using type_t = DWORD; // val.type
		using char_t = WCHAR;
		using int_t = int;
		using err_t = int;
		using rw_t = RW;
		using col_t = COL;
		using ref_rw_t = RW;
		using ref_col_t = COL;
		using ref_t = XLREF12;
		using idsheet_t = IDSHEET;
	};

	// Remove xlbit flags from type.
	constexpr auto xlbitFree = xlbitXLFree | xlbitDLLFree;
	template<is_xloper_t X>
	constexpr traits<X>::type_t type(const X& x) noexcept
	{
		return x.xltype & ~(xlbitFree);
	}
#ifdef _DEBUG
	static_assert(type(XLOPER{ .xltype = xltypeNum | xlbitXLFree }) == xltypeNum, "type() failed for XLOPER");
	static_assert(type(XLOPER12{ .xltype = xltypeNum | xlbitDLLFree }) == xltypeNum, "type() failed for XLOPER12");
#endif // _DEBUG

	// XLOPER type not using allocation.
	constexpr int xltypeScalar = xltypeNum | xltypeBool | xltypeErr
		| xltypeMissing | xltypeNil | xltypeSRef | xltypeInt;
	template<is_xloper_t X>
	constexpr bool isScalar(const X& x)
	{
		// xltypeBigData is xltypeStr | xltypeInt, but is not a scalar type.
		return type(x) == xltypeBigData ? false : type(x) & xltypeScalar;
	}

	// `return XLFree(x);` in thread-safe functions
	// Freed by Excel when no longer needed.
	template<is_xloper_t X>
	constexpr X* XLFree(X& x)
	{
		if (!isScalar(x)) {
			x.xltype |= xlbitXLFree;
			// TODO: does this work for xltypeBigData?
		}

		return &x;
	}

	// `return DLLFree(x);` in thread-safe functions
	// Excel calls xlAutoFree12 when no longer needed.
	template<is_xloper_t X>
	constexpr X* DLLFree(X& x)
	{
		if (!isScalar(x)) {
			x.xltype |= xlbitDLLFree;
		}

		return &x;
	}

	//
	// Freestanding functions for XLOPER/XLOPER12
	//

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
	// REF/XLREF12
	template<is_ref_t X>
	struct RefX : public traits<X>::ref_t {
		using rw = traits<X>::ref_rw_t;
		using col = traits<X>::ref_col_t;

		// Upper left corner (x, y) having width w and height h.
		constexpr RefX(rw x, col y, rw w, col h)
			: X({ x, x + w - 1, y + h - 1 })
		{ }
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

	//
	// XLOPER/XLOPER12
	// 
	// Size in cells of XLOPER
	template<is_xloper_t X>
	constexpr size_t rows(const X& x)
	{
		switch (type(x)) {
		case xltypeMissing: case xltypeNil:
			return 0;
		case xltypeRef:
			return x.val.mref->count;
		case xltypeMulti:
			return x.val.multi.rows;
		case xltypeSRef:
			return rows(x.val.sref);
		default:
			return 1;
		}
	}	// Size in cells
	template<is_xloper_t X>
	constexpr size_t columns(const X& x)
	{
		switch (type(x)) {
		case xltypeMissing: case xltypeNil:
			return 0;
		// case xltypeRef: return 1;
		case xltypeMulti:
			return x.val.multi.columns;
		case xltypeSRef:
			return columns(x.val.sref);
		default:
			return 1;
		}
	}	// Size in cells
	template<is_xloper_t X>
	constexpr size_t size(const X& x)
	{
		switch (type(x)) {
		case xltypeMissing: case xltypeNil:
			return 0;
		case xltypeRef:
			return size(x.val.mref->count);
		case xltypeMulti:
			return rows(x.val.multi) * columns(x.val.multi);
		case xltypeSRef:
			return size(x.val.sref);
		default:
			return 1; // TODO: xltypeBigData?
		}
	}

	template<is_xloper_t X>
	constexpr const X* begin(const X& x)
	{
		switch (type(x)) {
		case xltypeRef:
			return x.val.mref->lpmref;
		case xltypeMulti:
			return x.val.multi.lparray;
		default:
			return &x;
		}
	}
	template<is_xloper_t X>
	constexpr const X* end(const X& x)
	{
		switch (type(x)) {
		case xltypeRef:
			return x.val.mref->lpmref + x.val.mref->count;
		case xltypeMulti:
			return x.val.multi.lparray + size(x);
		default:
			return &x + 1;
		}
	}

	namespace {
		template<is_char_t C1, is_char_t C2>
		constexpr auto compare(const C1* b1, const C1* e1, const C2* b2, const C2* e2)
		{
			return std::lexicographical_compare_three_way(b1, e1, b2, e2);
		}
	}
#ifdef _DEBUG
	static_assert(compare("abc", "abc" + 3, "abc", "abc" + 3) == 0, "compare() failed");
	static_assert(compare("abc", "abc" + 3, "abd", "abd" + 3) < 0, "compare() failed");
	static_assert(compare("abd", "abd" + 3, "abc", "abc" + 3) > 0, "compare() failed");
	static_assert(compare(L"abc", L"abc" + 3, "abc", "abc" + 3) == 0, "compare() failed");
	static_assert(compare("abc", "abc" + 3, L"abd", L"abd" + 3) < 0, "compare() failed");
#endif // _DEBUG

	template<is_xloper_t X>
	constexpr bool equal(const X& lhs, const X& rhs)
	{
		if (type(lhs) != type(rhs)) {
			return false;
		}
		switch (type(lhs)) {
		case xltypeNum:
			return lhs.val.num == rhs.val.num;
		case xltypeStr:
			return compare(lhs.val.str[0], lhs.val.str + 1 + lhs.val.str[0],
					rhs.val.str + 1, rhs.val.str + 1 + rhs.val.str[0]) == 0;
		case xltypeBool:
			return lhs.val.xbool == rhs.val.xbool;
		case xltypeRef:
			return std::equal(begin(lhs), end(lhs), begin(rhs), end(rhs)); // TODO: compare each element?
		case xltypeErr:
			return lhs.val.err == rhs.val.err;
		case xltypeSRef:
			return equal(lhs.val.sref, rhs.val.sref);
		case xltypeMulti:
			return lhs.val.multi.rows == rhs.val.multi.rows and
				lhs.val.multi.columns == rhs.val.multi.columns and
				std::equal(begin(lhs), end(lhs), begin(rhs), end(rhs)); // TODO: compare each element?
		case xltypeMissing: case xltypeNil:
			return true;
		case xltypeSRef:
			return equal(lhs.val.sref, rhs.val.sref);
		case xltypeInt:
			return lhs.val.w == rhs.val.w;
		default:
			return false; // TODO: inequal??
		}
	}

	template<is_xloper_t X>
	constexpr std::partial_ordering compare(const X& lhs, const X& rhs)
	{
		if (type(lhs) != type(rhs)) {
			return type(lhs) <=> type(rhs);
		}
		switch (type(lhs)) {
		case xltypeNum:
			return lhs.val.num <=> rhs.val.num;
		case xltypeStr:
			return compare(lhs.val.str + 1, lhs.val.str + 1 + lhs.val.str[0],
					rhs.val.str + 1, rhs.val.str + 1 + rhs.val.str[0]) <=> 0;
		case xltypeBool:
			return lhs.val.xbool <=> rhs.val.xbool;
		case xltypeRef:
			return std::partial_ordering::unordered;
		case xltypeErr:
			return lhs.val.err <=> rhs.val.err;
		case xltypeMulti:
			return std::partial_ordering::unordered;
		case xltypeMissing: case xltypeNil:
			return 0 <=> 0;
		case xltypeSRef: // could be unordered.
			return std::partial_ordering::unordered;
		case xltypeInt:
			return lhs.val.w <=> rhs.val.w;
		default:
			return 0 <=> 1; // TODO: inequal??
		}
	}

	//
	// Scalar constructors for XLOPER and XLOPER12.
	//

	// Num (xltype = 1) defaults to NaN so that uninitialized XNum is not a valid number.
	template<is_xloper_t X>
	constexpr X NumX(double num = std::numeric_limits<double>::quiet_NaN())
	{
		return X{ .val{ .num = num }, .xltype = xltypeNum };
	}
	constexpr XLOPER Num4(double num = std::numeric_limits<double>::quiet_NaN())
	{
		return NumX<XLOPER>(num);
	}
	constexpr XLOPER12 Num(double num = std::numeric_limits<double>::quiet_NaN())
	{
		return NumX<XLOPER12>(num);
	}
#ifdef _DEBUG
	static_assert(type(Num()) == xltypeNum, "Num() failed");
	static_assert(compare(Num4(0), Num4(0)) == 0, "Num4() failed");
	static_assert(compare(Num(0), Num(0)) == 0, "Num() failed");
	static_assert(compare(Num(0), Num(1)) < 0, "Num() failed");
	static_assert(compare(Num(1), Num(0)) > 0, "Num() failed");
	static_assert(Num().val.num != Num().val.num, "Num() failed");	
#endif // _DEBUG

	// Str allocates

	// Bool (xltype == 4)
	template<is_xloper_t X>
	constexpr X BoolX(bool xbool)
	{
		return X{ .val{ .xbool = xbool }, .xltype = xltypeBool };
	}
	constexpr XLOPER Bool4(bool xbool)
	{
		return BoolX<XLOPER>(xbool);
	}
	constexpr XLOPER12 Bool(bool xbool)
	{
		return BoolX<XLOPER12>(xbool);
	}
#ifdef _DEBUG
	static_assert(type(Bool4(true)) == xltypeBool, "Bool4() failed");
	static_assert(type(Bool(true)) == xltypeBool, "Bool() failed");
	static_assert(compare(Bool4(true), Bool4(true)) == 0, "Bool4() failed");
	static_assert(compare(Bool(true), Bool(true)) == 0, "Bool() failed");
	static_assert(compare(Bool4(false), Bool4(true)) < 0, "Bool4() failed");
	static_assert(compare(Bool(false), Bool(true)) < 0, "Bool() failed");
	static_assert(compare(Bool4(true), Bool4(false)) > 0, "Bool4() failed");
	static_assert(compare(Bool(true), Bool(false)) > 0, "Bool() failed");
#endif // _DEBUG

	template<is_xloper_t X>
	constexpr X ErrX(typename traits<X>::err_t err)
	{
		return X{ .val{ .err = err }, .xltype = xltypeErr };
	}
	constexpr XLOPER Err4(WORD err)
	{
		return ErrX<XLOPER>(err);
	}
	constexpr XLOPER12 Err(int err)
	{
		return ErrX<XLOPER12>(err);
	}

	template<is_xloper_t X>
	constexpr X MissingX()
	{
		return X{ .xltype = xltypeMissing };
	}
	constexpr XLOPER Missing4()
	{
		return MissingX<XLOPER>();
	}
	constexpr XLOPER12 Missing()
	{
		return MissingX<XLOPER12>();
	}

	template<is_xloper_t X>
	constexpr X NilX()
	{
		return X{ .xltype = xltypeNil };
	}
	constexpr XLOPER Nil4()
	{
		return NilX<XLOPER>();
	}
	constexpr XLOPER12 Nil()
	{
		return NilX<XLOPER12>();
	}

	// Single cell reference constructor for XLOPER and XLOPER12.
	template<is_xloper_t X>
	constexpr traits<X>::ref_t REF(
		typename traits<X>::ref_rw_t rwFirst, 
		typename traits<X>::ref_rw_t rwLast,
		typename traits<X>::ref_col_t colFirst,
		typename traits<X>::ref_col_t colLast)
	{
		return X{ .rwFirst = rwFirst, .rwLast = rwLast,
				.colFirst = colFirst, .colLast = colLast };
	}
	// Single cell reference constructor for XLOPER and XLOPER12.
	template<is_xloper_t X>
	constexpr X SRef(const typename traits<X>::ref_t& ref)
	{
		return X{ X::val.count = 1, X::val.sref = ref, X::xltype = xltypeSRef };
	}

	template<is_xloper_t X>
	constexpr X Int(typename traits<X>::int_t w)
	{
		return X{ .val{ .w = w }, .xltype = xltypeInt };
	}


} // namespace xll

template<xll::is_xloper_t X>
constexpr auto operator<=>(const X& lhs, const X& rhs) 
{
	return xll::compare(lhs, rhs);
}