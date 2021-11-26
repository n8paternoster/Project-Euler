#include "stdafx.h"
#include "51-75/51.h"
#include <vector>
#include <unordered_set>

unsigned smallestPrimeOfNPrimeFamily(unsigned n) {

	const size_t maxN = 10;
	const size_t sieveSize = 1'000'000;

	if (n > maxN) n = maxN;
	
	// Generate a list of primes to iterate through and a sieve for fast lookup
	bool *primeSieve = new bool[sieveSize];
	for (size_t i = 0; i < sieveSize; i++) primeSieve[i] = true;
	std::vector<unsigned> primes;
	for (size_t i = 2; i < sieveSize; i++) {
		if (primeSieve[i]) {
			primes.push_back(i);
			for (size_t j = 2; i*j < sieveSize; j++) primeSieve[i*j] = false;
		}
	}

	// Bitmask indicates the index(es) of digits to be replaced
	std::vector<bool> bitMask;
	auto getNextBitMask = [&]() -> bool {
		size_t i = 1;			// the lsb (0) will never be a variable digit
		do bitMask[i].flip();	// increment the binary number (bitMask) by 1
		while (!bitMask[i] && ++i < bitMask.size());
		return (i != bitMask.size());
	};

	std::unordered_set<unsigned> solutions;
	unsigned returnVal = UINT_MAX;
	for (auto p : primes) {
		size_t numDigits = (size_t)log10(p)+1;
		if (numDigits < 2) continue;
		bitMask.clear();
		bitMask.resize(numDigits);
		while (getNextBitMask()) {
			size_t numVariableDigits = 0, numConstantDigits = 0;
			for (auto b : bitMask) b ? numVariableDigits++ : numConstantDigits++;

			unsigned protoNum = p, cDigit = 10;
			bool isDuplicate = false;
			for (unsigned i = 0; i < numDigits; i++) {
				if (bitMask[i]) {
					unsigned digit = (unsigned)(protoNum/pow(10, i)) % 10;
					protoNum -= digit*(unsigned)pow(10, i);
					if (cDigit > 9) cDigit = digit;
					else if (digit != cDigit) {
						isDuplicate = true;
						break;
					}
				}
			}
			if (isDuplicate) continue;


			// search for a family of numbers such that n values of x produce primes
			size_t numPrimes = 0;
			unsigned familyNumbers[maxN] = {0};
			for (unsigned x = bitMask[numDigits-1] ? 1 : 0; x < 10; x++) {
				if (10-x + numPrimes < n) break;

				// replace the digits in p at indexes indicated by bitMask with x
				unsigned num = protoNum;
				for (unsigned b = 1; b < numDigits; b++) {
					if (bitMask[b]) {
						//num -= ((unsigned)(num/pow(10, b))%10)*(unsigned)pow(10, b);
						num += (unsigned)(x*pow(10, b));
					}
				}
				if (num < sieveSize && primeSieve[num]) {
					numPrimes++;
					familyNumbers[x] = num;
				}
			}
			if (numPrimes >= n) {
				unsigned x = 0;
				while (familyNumbers[x] == 0) x++;
				if (!solutions.insert(familyNumbers[x]).second) continue;

				if (p < returnVal) returnVal = p;
				
				// print solution
				for (unsigned i = 0; i < numDigits; i++) {
					size_t index = numDigits-1-i;
					bitMask[index] ? std::cout << "*" : std::cout << (unsigned)(p/pow(10, index)) % 10;
				}
				std::cout << " produces " << numPrimes << " primes:\n";
				for (unsigned x = 0; x < maxN; x++)
					if (familyNumbers[x] != 0)
						std::cout << "\t" << familyNumbers[x] << "\n";
			}

			
		}
	}

	return returnVal;
}
