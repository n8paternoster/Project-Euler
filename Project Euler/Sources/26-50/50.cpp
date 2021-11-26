#include "stdafx.h"
#include "26-50/50.h"
#include <vector>

unsigned longestConsecutivePrimeSumBelowN(unsigned n) {

	// Generate all primes under n
	bool *primeSieve = new bool[n];
	for (unsigned i = 0; i < n; i++) primeSieve[i] = true;
	unsigned primeSum = 0;
	std::vector<unsigned> primes;	// store the maximum number of primes that can add up to < n
	for (unsigned i = 2; i < n; i++)
		if (primeSieve[i]) {
			if (primeSum < n) {
				primeSum += i;
				primes.push_back(i);
			}
			for (unsigned j = 2; i*j < n; j++) primeSieve[i*j] = false;
		}

	unsigned longestSeqLength = 0, longestSeqSum = 0, longestSeqStart = 0;
	unsigned seqSum = 0, seqStart;
	auto sumNPrimes = [&](unsigned n) {
		seqSum = 0;
		for (unsigned i = seqStart; i < seqStart+n; i++)
			seqSum += primes[i];
	};

	// Find the longest sequence that begins with each prime until there aren't enough primes left to find a longer one
	for (seqStart = 0; seqStart+longestSeqLength < primes.size(); seqStart++) {
		sumNPrimes(longestSeqLength);
		for (unsigned seqLength = longestSeqLength; seqStart + seqLength < primes.size(); seqLength++) {
			seqSum += primes[seqStart+seqLength];
			if (primeSieve[seqSum]) {
				longestSeqLength = seqLength+1;
				longestSeqSum = seqSum;
				longestSeqStart = seqStart;
			}
		}
	}
	
	// Print the sequence
	for (unsigned i = longestSeqStart; i < longestSeqLength; i++) {
		std::cout << primes[i];
		if (i+1 < longestSeqLength) std::cout << "+";
	}
	std::cout << " = " << longestSeqSum << " (" << longestSeqLength << " terms)\n";

	delete[] primeSieve;
	return longestSeqSum;
}
