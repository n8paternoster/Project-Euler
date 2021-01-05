#include "stdafx.h"
#include "21.h"
#include <math.h>

/*
Let d(n) be defined as the sum of proper divisors of n (numbers less than n which divide evenly into n).
If d(a) = b and d(b) = a, where a != b, then a and b are an amicable pair and each of a and b are called amicable numbers.

For example, the proper divisors of 220 are 1, 2, 4, 5, 10, 11, 20, 22, 44, 55 and 110; therefore d(220) = 284. The proper divisors of 284 are 1, 2, 4, 71 and 142; so d(284) = 220.

Evaluate the sum of all the amicable numbers under 10000.
*/

uint32_t sumOfDivisors(uint32_t n) {
	uint32_t sum = 1;	// every number is divisible by 1
	for (uint32_t i = 2; i <= sqrt(n); i++)
		if (n % i == 0) {
			sum += i;
			if (i != n / i) sum += n / i;
		}
	return sum;
}

uint32_t sumOfAmicableNumbersUnderN(uint32_t n) {
	uint32_t *divisorSums = new uint32_t[n+1]();
	uint32_t divisorSum = 0;
	uint32_t result = 0;
	for (uint32_t i = 2; i < n + 1; i++) {
		if (divisorSums[i] == 0) {
			divisorSum = sumOfDivisors(i);
			divisorSums[i] = divisorSum;
		} else divisorSum = divisorSums[i];
		if (divisorSum < n + 1) {
			if (divisorSums[i] == divisorSum && divisorSums[divisorSum] == i) {
				if (i == divisorSum || divisorSums[i] == 0) continue;
				// Amicable pair found (i, divisorSum)
				result += i + divisorSum;
			}
		}
	}
	return result;
}
