#pragma once

/*
	A palindromic number reads the same both ways. The largest palindrome made from the product of two 2-digit numbers is 9009 = 91 × 99.

	Find the largest palindrome made from the product of two 3-digit numbers.
*/

int largestPalindromeProductOfTwoXDigitNumbers(int numDigits);
inline auto p4() {
	return largestPalindromeProductOfTwoXDigitNumbers(3);
}