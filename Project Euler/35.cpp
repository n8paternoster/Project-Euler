#include "stdafx.h"
#include "35.h"

static bool isPrime(unsigned n) {
	if (n <= 3) return (n > 1);
	if (n % 2 == 0 || n % 3 == 0) return false;
	for (unsigned i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
		if (n % i == 0 || n % (i + 2) == 0) return false;
	return true;
}

unsigned numCircularPrimesUnderN(unsigned N) {

	unsigned counter = 1;	// the first prime, 2, is circular
	for (unsigned n = 3; n < N; n += 2) {
		if (isPrime(n)) {
			// Check all rotations
			unsigned m = n;
			unsigned numDigits = (unsigned)log10(n)+1;
			bool allPrime = true;
			for (unsigned i = 0; i < numDigits; i++) {
				unsigned digit = m % 10;
				m = m / 10 + digit * pow(10, numDigits-1);
				if (!isPrime(m)) {
					allPrime = false;
					break;
				}
			}
			if (allPrime) counter++;
		}
	}
	return counter;
}
