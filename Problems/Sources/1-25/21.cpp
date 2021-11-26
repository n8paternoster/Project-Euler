#include "pch.h"
#include "1-25/21.h"
#include <math.h>

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
