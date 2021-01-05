#include "stdafx.h"
#include "12.h"

/*
The sequence of triangle numbers is generated by adding the natural numbers. So the 7th triangle number would be 1 + 2 + 3 + 4 + 5 + 6 + 7 = 28. The first ten terms would be:

1, 3, 6, 10, 15, 21, 28, 36, 45, 55, ...

Let us list the factors of the first seven triangle numbers:

1: 1
3: 1,3
6: 1,2,3,6
10: 1,2,5,10
15: 1,3,5,15
21: 1,3,7,21
28: 1,2,4,7,14,28
We can see that 28 is the first triangle number to have over five divisors.

What is the value of the first triangle number to have over five hundred divisors?
*/


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