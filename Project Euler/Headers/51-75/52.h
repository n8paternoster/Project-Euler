#pragma once

/*
	It can be seen that the number, 125874, and its double, 251748, contain exactly the same digits, but in a different order.

	Find the smallest positive integer, x, such that 2x, 3x, 4x, 5x, and 6x, contain the same digits.
*/

unsigned smallestPermutatedNMultiple(unsigned n);
inline auto p52() {
	return smallestPermutatedNMultiple(6);
}