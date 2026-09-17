// xloper.h - XLOPER/XLOPER12 functions
#pragma once
#include "xltraits.h"

namespace xll {

	// Size in cells of XLOPER/XLREF
	template<is_xloper_t X>
	constexpr xloper_traits<X>::row_t rows(const X& x)
	{
		switch (type(x)) {
		case xltypeMissing: case xltypeNil:
			return 0;
		case xltypeMulti:
			return x.val.multi.rows;
		case xltypeSRef:
			return rows(x.val.sref);
		default:
			return 1;
		}
	}	
	template<is_xloper_t X>
	constexpr xloper_traits<X>::col_t columns(const X& x)
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
	constexpr auto size(const X& x)
	{
		return rows(x) * columns(x);
	}

	template<is_xloper_t X>
	constexpr const X* begin(const X& x)
	{
		return xltype(x) == xltypeMulti ? x.val.multi : &x;
	}
	template<is_xloper_t X>
	constexpr const X* end(const X& x)
	{
		return xltype(x) == xltypeMulti ? x.val.multi + size(x) : &x + 1;
	}

	template<is_xloper_t X>
	constexpr bool equal(const X& lhs, const X& rhs)
	{
		if (xltype(lhs) != xltype(rhs)) {
			return false;
		}
		switch (xltype(lhs)) {
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
					lhs.val.mref->lpmref, lhs.val.mref->array + lhs.val.mref->count,
					rhs.val.mref->lpmref, rhs.val.mref->array + rhs.val.mref->count);
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
		return X{ .val{.num = num }, .xltype = xltypeNum };
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
		return X{ .val{.xbool = xbool }, .xltype = xltypeBool };
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
		return X{ .val{.err = err }, .xltype = xltypeErr };
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
		return X{ .val{.w = w }, .xltype = xltypeInt };
	}


} // namespace xll
