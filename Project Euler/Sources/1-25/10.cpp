#include "stdafx.h"
#include "1-25/10.h"
#include <vector>

// Using the Sieve of Eratosthenes with the sieve size = 2,000,000
uint64_t summationOfPrimesBelowN(uint64_t n) {
	const unsigned sieveSize = n+1;
	bool *primeSieve = new bool[sieveSize]();	// true if the index is prime
	for (unsigned i = 2; i < sieveSize; i++) primeSieve[i] = true;
	for (unsigned i = 2; i < sieveSize; i++)
		if (primeSieve[i])
			for (unsigned j = 2; i*j < sieveSize; j++) primeSieve[i*j] = false;
	uint64_t primeSum = 0;
	for (unsigned i = 0; i < sieveSize; i++)
		if (primeSieve[i]) primeSum += i;
	delete[] primeSieve;
	return primeSum;
}
