#include "stdafx.h"
#include "3.h"

/*
The prime factors of 13195 are 5, 7, 13 and 29.

What is the largest prime factor of the number 600851475143 ?
*/


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