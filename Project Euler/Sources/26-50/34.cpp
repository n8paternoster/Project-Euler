#include "stdafx.h"
#include "26-50/34.h"

unsigned factorial(unsigned n) {
	return (n == 1 || n == 0) ? 1 : factorial(n-1)*n;
}

unsigned sumOfDigitFactorialNumbers() {
	unsigned sum = 0;
	for (unsigned n = 3; n <= factorial(9)*7; n++) {
		unsigned m = n;
		unsigned factorialSum = 0;
		for (unsigned i = 0; i < (unsigned)log10(n)+1; i++) {
			factorialSum += factorial(m % 10);
			m /= 10;
		}
		if (factorialSum == n) sum += n;
	}
	return sum;
}
