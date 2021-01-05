#include "stdafx.h"
#include "45.h"

static bool isPentagonal(unsigned long long x) {
	unsigned long long root = (unsigned long long)sqrt(x*24+1);
	bool isSquare = (x*24+1) == root*root || (x*24+1) == (root+1)*(root+1);	// for floating-point error
	return (isSquare && (root+1)%6 == 0);
}

// Every hexagonal number is a triangle number, only need to generate hexagonal numbers and check if they are pentagonal
unsigned long long nthTriangularPentagonalHexagonalNumber(unsigned n) {

	unsigned solutionCount = 0;
	unsigned long long hex, h = 1;
	while (solutionCount < n) {
		hex = 2*h*h - h;
		if (isPentagonal(hex)) solutionCount++;
		h++;
	}

	return hex;
}
