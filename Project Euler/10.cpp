#include "stdafx.h"
#include "10.h"
#include <vector>

/*
The sum of the primes below 10 is 2 + 3 + 5 + 7 = 17.

Find the sum of all the primes below two million.
*/

// Using the Sieve of Eratosthenes with the sieve size = 2,000,000
uint64_t summationOfPrimesBelowN(uint64_t n) {
	uint64_t sieveSize = n + 1;
	std::vector<bool> sieve(sieveSize, true);	// 1-indexed, index directly corresponds to number
	sieve[0] = false;	// 0 and 1 are not primes
	sieve[1] = false;
	uint64_t primeSum = 0;
	uint64_t prime = 0;
	while (prime < n) {

		// find the next prime
		do {
			prime++;				
			if (prime > n) return primeSum;
		} while (!sieve[prime]);

		// remove all multiples of this prime as possible primes
		for (uint64_t i = prime + prime; i < sieveSize; i += prime)
			if (sieve[i]) sieve[i] = false;	

		// add this prime to the sum of primes
		primeSum += prime;
	}
	return primeSum;
}
