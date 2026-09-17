// traits.h - Specialize old and new Excel data types
// Top level include.
#pragma once
#define WINDOWS_LEAN_AND_MEAN                   
#include <Windows.h>
#include "XLCALL.H"
#include <concepts>

namespace xll {

	// Excel character types for xltypeStr.
	template<class C>
	concept is_char_t = std::same_as<C, CHAR> || std::same_as<C, XCHAR>;

	// Single reference to range.
	template<class R>
	concept is_ref_t = std::same_as<R, XLREF> || std::same_as<R, XLREF12>;
	// Allows for top level functions that can be used by subclasses.
	template<class R>
	concept convertible_to_ref_t = std::convertible_to<R, XLREF> || std::convertible_to<R, XLREF12>;

	// Multiple range references
	template<class R>
	concept is_mref_t = std::same_as<R, XLMREF> || std::same_as<R, XLMREF12>;

	template<class X>
	concept is_xloper_t = std::same_as<X, XLOPER> || std::same_as<X, XLOPER12>;
	template<class X>
	concept convertible_to_xloper_t = std::convertible_to<X, XLOPER> || std::convertible_to<X, XLOPER12>;

	// Specialize for REF/REF12
	template<is_ref_t R>
	struct ref_traits {
		using rw_t = typename decltype(R::rwFirst);
		using col_t = typename decltype(R::colFirst);
	};

	// Specialize for XLOPER/XLOPER12
	template<is_xloper_t X>
	struct xloper_traits {
		using xltype_t = typename decltype(X::xltype);
		using str_t = decltype(X::val.str);
		using char_t = decltype(*X::val.str);
		using int_t = decltype(X::val.w);
		using err_t = decltype(X::val.err); // val.err
		using rw_t = decltype(X::val.array.rows);
		using col_t = decltype(X::val.array.columns);
		using ref_t = decltype(X::val.sref);
		using mref_t = decltype(*X::val.mref.lpmref);
	};

	// Remove xlbit flags from type.
	constexpr WORD xlbitFree = xlbitXLFree | xlbitDLLFree;
	template<is_xloper_t X>
	constexpr xloper_traits<X>::xltype_t xltype(const X& x) noexcept
	{
		return x.xltype & ~(xlbitFree);
	}
#ifdef _DEBUG
	static_assert(xltype(XLOPER{ .xltype = xltypeNum | xlbitXLFree }) == xltypeNum, "type() failed for XLOPER");
	static_assert(xltype(XLOPER12{ .xltype = xltypeNum | xlbitDLLFree }) == xltypeNum, "type() failed for XLOPER12");
#endif // _DEBUG

	// XLOPER types not using allocation.
	constexpr int xltypeScalar = xltypeNum | xltypeBool | xltypeErr
		| xltypeMissing | xltypeNil | xltypeSRef | xltypeInt;
	template<is_xloper_t X>
	constexpr bool isScalar(const X& x)
	{
		// xltypeBigData is xltypeStr | xltypeInt, but is not a scalar type.
		return type(x) == xltypeBigData ? false : type(x) & xltypeScalar;
	}

	// `return XLFree(x);` in thread-safe functions if x owned by Excel
	// Freed by Excel when no longer needed.
	template<is_xloper_t X>
	constexpr X* XLFree(X& x)
	{
		if (type(x) == xltypeMulti) {
			x.xltype |= xlbitXLFree;
			// TODO: does this work for xltypeBigData?
		}

		return &x;
	}

	// `return DLLFree(x);` in thread-safe functions if x owned by you
	//  Freed by xlAutoFree/xlAutoFree12 when no longer needed.
	template<is_xloper_t X>
	constexpr X* DLLFree(X& x)
	{
		if (type(x) == xltypeMulti) {
			x.xltype |= xlbitDLLFree;
		}

		return &x;
	}
#if 0
	//
	// Freestanding functions for XLOPER/XLOPER12
	//

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
	}	
	// Size in cells
	template<is_xloper_t X>
	constexpr size_t size(const X& x)
	{
		return rows(x) * columns(x);
	}

	template<is_xloper_t X>
	constexpr const X* begin(const X& x)
	{
		switch (type(x)) {
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
		case xltypeMulti:
			return begin(x) + size(x);
		default:
			return &x + 1;
		}
	}

	template<is_xloper_t X>
	constexpr bool equal(
		typename const traits<X>::mref_t& lhs,
		typename const traits<X>::mref_t& rhs)
	{
		const traits<X>::mref_t* lref = lhs.val.mref.lpmref;
		const traits<X>::mref_t* rref = rhs.val.mref.lpmref;
		return std::equal(
			lref->reftbl, lref->reftbl + lref->count,
			rref->reftbl, rref->reftbl + rref->count);
	}

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
			return std::equal(
					lhs.val.str + 1, lhs.val.str + 1 + lhs.val.str[0],
					rhs.val.str + 1, rhs.val.str + 1 + rhs.val.str[0]) == 0;
		case xltypeBool:
			return lhs.val.xbool == rhs.val.xbool;
		case xltypeRef:
			return lhs.val.mref.idSheet == rhs.val.mref.idSheet
				and std::equal(
					lhs.val.mref->lpmref, lhs.val.mref->array + lhs.val.mref	,					begin(rhs), end(rhs)); // TODO: compare each element?
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
					rhs.val.str + 1, rhs.val.str + 1 + rhs.val.str[0]);
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
#endif // 0

} // namespace xll
/*
template<xll::is_xloper_t X>
constexpr auto operator<=>(const X& lhs, const X& rhs) 
{
	return xll::compare(lhs, rhs);
}
*/