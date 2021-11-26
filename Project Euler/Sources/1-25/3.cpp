#include "stdafx.h"
#include "1-25/3.h"

uint64_t largestPrimeFactorOf(uint64_t n) {
	uint64_t largestPF = 1;
	uint64_t i = 2;
	while (i < n/2) {
		if (n % i == 0) {
			if (i > largestPF) largestPF = i;
			n /= i;
			i = 2;
			continue;
		}
		i++;
	}
	if (n > largestPF) largestPF = n;
	return largestPF;
}