#include "stdafx.h"
#include "24.h"
#include <sstream>

/*
A permutation is an ordered arrangement of objects.For example, 3124 is one possible permutation of the digits 1, 2, 3 and 4. If all of the permutations are listed numerically or alphabetically, we call it lexicographic order.The lexicographic permutations of 0, 1 and 2 are:

012   021   102   120   201   210

What is the millionth lexicographic permutation of the digits 0, 1, 2, 3, 4, 5, 6, 7, 8 and 9 ?
*/

const uint64_t numDigits = 10;

uint64_t factorial(uint64_t n) {
	if (n <= 1)
		return 1;
	return n * factorial(n - 1);
}

std::string nthLexicographicPermutation(uint64_t n) {
	if (n < 1 || n > factorial(numDigits)) {
		std::cerr << "Invalid number\n";
		return "";
	}
	uint64_t permutation[numDigits];			// The nth permutation
	bool unusedDigits[numDigits];
	for (uint64_t i = 0; i < numDigits; i++) 
		unusedDigits[i] = true;

	// Generate each digit in order
	uint64_t digit, fact;
	for (uint64_t i = 0; i < numDigits; i++) {
		fact = factorial(numDigits - 1 - i);	// With the ith digit set, there are (i-1)! possible permutations
		digit = (n - 1) / fact;					// Gives the ith lexicographic digit
		
		// Find which remaining unused digit is the ith digit
		// e.g. If digit = 2 but the number 1 was already used
		// then the ith digit will be 3
		uint64_t d = 0;
		int64_t k = -1;
		while (true) {
			do k++;
			while (!unusedDigits[k]);
			if (d == digit) break;
			else d++;
		}
		unusedDigits[k] = false;
		permutation[i] = k;
		n -= digit * fact;
	}

	// Convert permutation into a string
	std::stringstream ss;
	for (uint64_t i : permutation)
		ss << i;

	return ss.str();
}