#pragma once

/*
	2^15 = 32768 and the sum of its digits is 3 + 2 + 7 + 6 + 8 = 26.

	What is the sum of the digits of the number 2^1000?
*/

int digitSumOfXRaisedToY(int, int);
inline auto p16() {
	return digitSumOfXRaisedToY(2, 1000);
}