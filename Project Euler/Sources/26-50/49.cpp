#include "stdafx.h"
#include "26-50/49.h"
#include <string>

bool isPermutation(unsigned x, unsigned y) {
	int digits[10] = {0};
	while (x != 0) {
		digits[x%10]++;
		x /= 10;
	}
	while (y != 0) {
		digits[y%10]--;
		y /= 10;
	}
	for (unsigned i = 0; i < 10; i++)
		if (digits[i] != 0) return false;
	return true;
}

std::string primePermutationSequence() {

	std::string returnVal("");

	// Generate the primes up to 9999
	const unsigned sieveSize = 10000;
	bool primes[sieveSize];
	for (unsigned i = 0; i < sieveSize; i++) primes[i] = true;
	for (unsigned i = 2; i < sieveSize; i++)
		if (primes[i])
			for (unsigned j = 2; i*j < sieveSize; j++) primes[i*j] = false;

	// Search for two primes that are permutations of each other and then check if the sum of the primes is also prime and a permutation of both
	for (unsigned a = 2; a < sieveSize; a++) {
		if (primes[a]) {
			for (unsigned b = a+1; (b-a)+b < sieveSize; b++) {
				if (primes[b] && isPermutation(a, b) && primes[(b-a)+b] && isPermutation((b-a)+b, a)) {
					std::cout << "a=" << a << " b=" << b << " c=" << (b-a)+b << " Difference=" << b-a << "\n";
					if (a > 1000 && a < 10000 && a != 1487) returnVal = std::to_string(a) + std::to_string(b) + std::to_string((b-a)+b);
				}
			}
		}
	}

	return returnVal;
}
