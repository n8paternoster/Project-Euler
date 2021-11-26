#pragma once

/*
	It is possible to show that the square root of two can be expressed as an infinite continued fraction.

	By expanding this for the first four iterations, we get:

	1 + 1/2 = 3/2
	1 + 1/(2+1/2) = 7/5
	1 + 1/(2+(1/(2+1/2))) = 17/12
	1 + 1/(2+(1/(2+(1/(2+1/2))))) = 41/29

	The next three expansions are 99/70, 239/169, and 577/408, but the eighth expansion, 1393/985, is the first example where the number of digits in the numerator exceeds the number of digits in the denominator.

	In the first one-thousand expansions, how many fractions contain a numerator with more digits than the denominator?
*/

unsigned numConvergentsWithLargerNumerator(unsigned numExpansions);
inline auto p57() {
	return numConvergentsWithLargerNumerator(1000);
}