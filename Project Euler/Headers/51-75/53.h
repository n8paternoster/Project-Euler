#pragma once

/*
	There are exactly ten ways of selecting three from five, 12345:

	123, 124, 125, 134, 135, 145, 234, 235, 245, and 345

	In combinatorics, we use the notation, (5, 3) = 10.

	In general, (n, r) = n! / r!(n-r)!, where r <= n, n! = n x (n - 1) x ... x 3 x 2 x 1, and 0! = 1

	It is not until n = 23, that a value exceeds one-million: (23, 10) = 1144066.

	How many, not necessarily distinct, values of (n, r) for 1 <= n <= 100 are greater than one-million?
*/

unsigned numCombinationsAboveOneMillion(unsigned n);
inline auto p53() {
	return numCombinationsAboveOneMillion(100);
}