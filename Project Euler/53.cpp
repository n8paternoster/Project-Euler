#include "stdafx.h"
#include "53.h"


// (n, r), [n choose r], is symmetrical such that (n, r) = (n, n-r). Therefore we only need to check values of r up to n/2. The number of combinations for a given n and r is calculated by alternating operations between multiplying by a term in the numerator and dividing by a term in the denominator. This converges to the correct value and avoids integer overflow. We can stop this process once the partial result is > 1,000,000 after a division operation since it will only further increase.
unsigned numCombinationsAboveOneMillion(unsigned N) {
	if (N < 23) return 0;

	unsigned solutionCount = 0;
	for (unsigned n = 23; n <= N; n++) {
		unsigned solutionsForN = 0;		// used to account for nCr symmetry
		for (unsigned r = 1; r <= n/2; r++) {
			unsigned num = n, den = r;
			float numCombinations = 1.0f;
			while (num > n-r && den > 0) {
				numCombinations *= num--;
				numCombinations /= den--;
				if (round(numCombinations) > 1'000'000.0f) break;
			}
			if (round(numCombinations) > 1'000'000.0f) solutionsForN++;
		}
		if (solutionsForN > 0)
			solutionCount += (n % 2 == 0) ? solutionsForN*2-1 : solutionsForN*2;
	}
	return solutionCount;
}
