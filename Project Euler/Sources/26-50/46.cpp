#include "stdafx.h"
#include "26-50/46.h"

bool isPrime(unsigned n) {
	if (n <= 3) return (n > 1);
	if (n % 2 == 0 || n % 3 == 0) return false;
	for (unsigned i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
		if (n % i == 0 || n % (i + 2) == 0) return false;
	return true;
}

bool isSquare(unsigned n) {
	unsigned root = (unsigned)sqrt(n);
	return (n == root*root || n == (root+1)*(root+1));	// for floating-point error
}

unsigned goldBachSmallestCounterexample() {

	for (unsigned n = 3; ; n += 2) {
		if (isPrime(n)) continue;
		bool hasSolution = false;
		for (unsigned p = 3; p < n; p += 2) {
			if (isPrime(p) && isSquare((n-p)/2)) {
				hasSolution = true;
				break;
			}
		}
		if (!hasSolution) return n;
	}

	return 0;
}
