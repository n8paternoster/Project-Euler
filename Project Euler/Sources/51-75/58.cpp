#include "stdafx.h"
#include "51-75/58.h"

static const unsigned maxSideLength = 50000u;

static bool isPrime(unsigned n) {
	if (n <= 3) return (n > 1);
	if (n % 2 == 0 || n % 3 == 0) return false;
	for (unsigned i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
		if (n % i == 0 || n % (i + 2) == 0) return false;
	return true;
}

unsigned sideLengthForDiagonalPrimeRatioBelow(double ratio) {
    unsigned numPrimes = 0;         // ratio is numPrimes / numDiagonals
    unsigned numDiagonals = 1;
	unsigned sideLength = 3;
	while (sideLength < maxSideLength) {
		numDiagonals += 4;
		unsigned n = sideLength * sideLength;
		for (unsigned i = 0; i < 3; i++) {	
			n -= (sideLength-1);	// n itself is never prime
			if (isPrime(n)) numPrimes++;
		}
		if (static_cast<double>(numPrimes) / numDiagonals < ratio) return sideLength;
		sideLength += 2;
	}

    return 0;
}
