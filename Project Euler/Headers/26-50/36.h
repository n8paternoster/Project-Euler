#pragma once

/*
	The decimal number, 585 = 10010010012 (binary), is palindromic in both bases.

	Find the sum of all numbers, less than one million, which are palindromic in base 10 and base 2.

	(Please note that the palindromic number, in either base, may not include leading zeros.)
*/

unsigned sumDoubleBasePalindromesUnderN(unsigned N);
inline auto p36() {
	return sumDoubleBasePalindromesUnderN(1'000'000);
}