#include "pch.h"
#include "1-25/7.h"
#include <vector>

unsigned long nthPrimeNumber(int n) {
	unsigned long sieveSize = 1000000;
	std::vector<bool> sieve(sieveSize, true);	// 1-indexed, index directly corresponds to number
	sieve[0] = false;
	sieve[1] = false;
	int numPrimes = 0;			// tracks the number of primes
	unsigned long prime = 0;	// will hold the value of a prime number
	while (numPrimes < n) {
		if (prime == sieveSize-1) {
			std::cout << "Prime out of sieve range.\n";
			return 0;
		}
		do prime++;				// find the next prime
		while (!sieve[prime]);
		numPrimes++;			// increment the number of primes found
		for (unsigned long i = prime + prime; i < sieveSize-1; i += prime)	// remove all multiples of this prime as possible primes
			if (sieve[i]) sieve[i] = false;
	}
	return prime;
}
