#include "stdafx.h"
#include "1-25/12.h"

/* 
	Function to count the number of divisors of n. The number of divisors can be calculated from the prime factors. Find all prime factors in the form x^y, add 1 to each exponent y, and multiply them together.
*/
int numOfDivisors(uint64_t n) {
	int numDivisors = 1;
	int exp = 0;			// counts the exponent of a prime factor (e.g. the "3" if the prime factorization includes
							// the term 2^3)
	for (uint64_t factor = 2; factor < (n / 2) + 1 && n > 1; factor++) {
		if (n % factor == 0) {
			exp = 0;
			while (n % factor == 0) {
				n /= factor;
				exp++;
			}
			numDivisors *= exp + 1;
		}
	}
	if (n > 1) numDivisors *= 2;	// the final prime factor only appears once
	return numDivisors;
}

uint64_t firstTriangleNumberOverNDivisors(int n) {
	uint64_t triangleNum = 0;
	for (uint64_t i = 1; i < UINT64_MAX; i++) {
		triangleNum += i;
		if (numOfDivisors(triangleNum) >= n) return triangleNum;
	}
	std::cerr << "Number of divisors too large\n";
	return -1;
}