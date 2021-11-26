#include "stdafx.h"
#include "1-25/6.h"

uint32_t sumSquareDifferenceOfFirstXNaturalNumbers(uint32_t n) {
	uint32_t sumOfSquares = 0;
	uint32_t squareOfSums = 0;
	for (uint32_t i = 1; i <= n; i++) {
		sumOfSquares += i*i;
		squareOfSums += i;
	}
	squareOfSums *= squareOfSums;
	return squareOfSums - sumOfSquares;
}
