#include "stdafx.h"
#include "26.h"

uint32_t longestRecurringDecimalUnderDivisorN(uint32_t N) {
	bigNum one("1");
	bigNum n, quotient;
	uint32_t longestN = 1;
	uint32_t longestPeriod = 0;

	for (uint32_t i = 1; i <= N; i++) {
		n = bigNum(std::to_string(i));
		divideWithPrecision(one, n, quotient, i);
		if (quotient.repetendPeriod() > longestPeriod) {
			longestPeriod = quotient.repetendPeriod();
			longestN = i;
		}
		std::cout << "1/" << i << ": ";
		quotient.print();
	}

	return longestN;
}