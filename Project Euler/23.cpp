#include "stdafx.h"
#include "23.h"
#include <vector>

/*
A perfect number is a number for which the sum of its proper divisors is exactly equal to the number. For example, the sum of the proper divisors of 28 would be 1 + 2 + 4 + 7 + 14 = 28, which means that 28 is a perfect number.

A number n is called deficient if the sum of its proper divisors is less than n and it is called abundant if this sum exceeds n.

As 12 is the smallest abundant number, 1 + 2 + 3 + 4 + 6 = 16, the smallest number that can be written as the sum of two abundant numbers is 24. By mathematical analysis, it can be shown that all integers greater than 28123 can be written as the sum of two abundant numbers. However, this upper limit cannot be reduced any further by analysis even though it is known that the greatest number that cannot be expressed as the sum of two abundant numbers is less than this limit.

Find the sum of all the positive integers which cannot be written as the sum of two abundant numbers.
*/

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
	bool isSumOfAbundants[max + 1] = {false};

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

	return result;
}