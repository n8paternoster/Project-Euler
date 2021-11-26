#include "pch.h"
#include "1-25/5.h"

uint32_t smallestNumberDivisibleBy1To20() {
	uint32_t n = 20;
	for (; n < UINT32_MAX; n += 20) {
		// Don't need to check 1, 2, 4, 5, 10, 20
		// With % 18 check: don't need to check 3, 6, 9
		// With % 16 check: don't need to check 8
		// With % 14 check: don't need to check 7
		// Remaining checks are: 11, 12, 13, 15, 17, 19
		if (n % 11 == 0 && n % 12 == 0 && n % 13 == 0 && n % 14 == 0 && n % 15 == 0 && n % 16 == 0 && n % 17 == 0 && n % 18 == 0 && n % 19 == 0)
			return n;
	}
	return 0;
}
