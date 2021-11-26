#include "pch.h"
#include "1-25/23.h"
#include <vector>

const int max = 28123;

bool isAbundant(uint32_t n) {
	uint32_t divisorSum = 1;	// 1 is always a divisor
	for (uint32_t i = 2; i*i <= n; i++)
		if (n % i == 0) {
			divisorSum += i;
			if (i != n / i) divisorSum += n / i;	// if n is a perfect square, don't include sqrt(n) twice
		}
	return divisorSum > n;
}

uint32_t sumOfNonAbundantSums() {

	std::vector<uint32_t> abundantNums;
	bool *isSumOfAbundants = new bool[max + 1]();

	for (uint32_t i = 1; i <= max; i++) {
		// Find all abundant numbers from 1 - max
		if (isAbundant(i))
			abundantNums.push_back(i);
	}

	// Find all possible sums of 2 abundant numbers
	uint32_t x, y;
	for (uint32_t a = 0; a < abundantNums.size(); a++) {
		for (uint32_t b = a; b < abundantNums.size(); b++) {
			x = abundantNums[a];
			y = abundantNums[b];
			if (x + y <= max) isSumOfAbundants[x + y] = true;
		}
	}

	uint32_t result = 0;
	for (uint32_t i = 0; i <= max; i++)
		if (!isSumOfAbundants[i]) result += i;

	delete[] isSumOfAbundants;

	return result;
}