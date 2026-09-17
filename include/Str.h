// Str.h - xltypeStr
#pragma once
#include "xloper.h"

namespace xll {
	template<class X>
	struct XStr : public X {
		using char_t = typename traits<X>::char_t;
		XStr(const char_t* str = nullptr)
			: X::xltype(xltypeStr)
		{
			if (str) {
				size_t len = wcslen(str);
				X::val.str = new char_t[len + 1];
				wcscpy_s(X::val.str, len + 1, str);
			}
			else {
				X::val.str = "0";
			}
		}
		~XStr()
		{
			if (X::xltype == xltypeStr && X::val.str) {
				delete[] X::val.str;
			}
		}
		operator const char_t* () const { return X::val.str; }
	};
	using Str4 = XStr<XLOPER>;
	using Str = XStr<XLOPER12>;
} // namespace xll