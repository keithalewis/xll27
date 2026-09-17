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

#endif // 0

} // namespace xll
/*
template<xll::is_xloper_t X>
constexpr auto operator<=>(const X& lhs, const X& rhs) 
{
	return xll::compare(lhs, rhs);
}
*/