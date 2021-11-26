#include "pch.h"
#include "26-50/26.h"
#include "bigNum.h"

uint32_t longestRecurringDecimalUnderDivisorN(uint32_t N) {
	bigNum::setPrecision(N);

	uint32_t longestN = 1;
	uint32_t longestPeriod = 0;

	bigNum one("1");
	for (uint32_t i = N; i > 0; i--) {
		if (i < longestPeriod) break;
		bigNum n = bigNum(std::to_string(i));
		bigNum quotient = one / n;
		if (quotient.repPeriod() > longestPeriod) {
			longestPeriod = quotient.repPeriod();
			longestN = i;
		}
		std::cout << "1/" << i << ": ";
		quotient.print();
	}

	return longestN;
}