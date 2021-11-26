#include "stdafx.h"
#include "26-50/47.h"

unsigned firstQuartetDistinctPrimeFactors() {

	const unsigned sieveSize = 150000;
	unsigned numDistinct[sieveSize] = {0};

	// Sieve through the primes
	for (unsigned i = 2; i < sieveSize; i++)	
		if (numDistinct[i] == 0)
			for (unsigned j = 2; i*j < sieveSize; j++) numDistinct[i*j]++;

	// Search for 4 in a row
	unsigned a;
	for (a = 2; a < sieveSize-4; a++)
		if (numDistinct[a] == 4 && numDistinct[a+1] == 4 && numDistinct[a+2] == 4 && numDistinct[a+3] == 4) return a;

	return 0;
}
