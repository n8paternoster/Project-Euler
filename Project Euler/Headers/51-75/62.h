#pragma once

/*
	The cube, 41063625 (345^3), can be permuted to produce two other cubes: 56623104 (384^3) and 66430125 (405^3). In fact, 41063625 is the smallest cube which has exactly three permutations of its digits which are also cube.

	Find the smallest cube for which exactly five permutations of its digits are cube.
*/

unsigned long long smallestCubeWithNCubicPermutations(unsigned N, unsigned long long maxBase = 2'600'000ull);
inline auto p62() {
	return smallestCubeWithNCubicPermutations(5, 10000);	// 10,000 needed for group of 5
}