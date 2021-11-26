#include "stdafx.h"
#include "26-50/30.h"

unsigned sumOfDigitNPowerNumbers(unsigned n) {
	unsigned result = 0, max = 0;
	for (unsigned i = 0; i < n+1; i++)
		max += pow(9, n);
	for (unsigned num = 2; num < max; num++) {
		unsigned sum = 0;
		for (unsigned d = 0; d <= (unsigned)log10(num); d++) {
			unsigned digit = (unsigned)(num/pow(10, d)) % 10;
			sum += pow(digit, n);
		}
		if (sum == num) result += sum;
	}
	
	return result;
}