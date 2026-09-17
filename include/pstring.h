#pragma once

#include <array>
#include <cstddef>

namespace xll {

	// Turns a string literal "foo" (size N = 4, includes '\0')
	// into { 3, 'f', 'o', 'o' } -- i.e., "\0x3foo" -- a Pascal-style,
	// length-prefixed, non-null-terminated string as used by XLOPER/XLOPER12.
	template<std::size_t N>
	constexpr std::array<char, N> make_pstring(const char (&str)[N])
	{
		std::array<char, N> result{};
		result[0] = static_cast<char>(N - 1); // length, excluding the null terminator

		for (std::size_t i = 0; i < N - 1; ++i) {
			result[i + 1] = str[i];
		}

		return result;
	}

	template<typename X, X... chars>
	constexpr auto operator""_pstr()
	{
		constexpr char str[] = { chars..., '\0' };

		return make_pstring(str);
	}
	static_assert(make_pstring("foo") == "\x3foo", "make_pstring failed");

}
