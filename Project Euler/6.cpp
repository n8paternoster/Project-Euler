#include "stdafx.h"
#include "6.h"

/*
The sum of the squares of the first ten natural numbers is,
	1^2+2^2+...+10^2 = 385
The square of the sum of the first ten natural numbers is,
	(1+2+...+10)^2 = 55^2 = 3025
Hence the difference between the sum of the squares of the first ten natural numbers and the square of the sum is 3025-385 = 2640
Find the difference between the sum of the squares of the first one hundred natural numbers and the square of the sum.
*/

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
