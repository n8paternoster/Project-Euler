#include "stdafx.h"
#include "26-50/45.h"

static bool isPentagonal(unsigned long long x) {
	unsigned long long root = (unsigned long long)sqrt(x*24+1);
	bool isSquare = (x*24+1) == root*root || (x*24+1) == (root+1)*(root+1);	// for floating-point error
	return (isSquare && (root+1)%6 == 0);
}

// Every hexagonal number is a triangle number, only need to generate hexagonal numbers and check if they are pentagonal
unsigned long long nextTriangularPentagonalHexagonalNumberAfter(unsigned n) {

	// Check that n is a hexagonal number
	unsigned long long root = (unsigned long long)sqrt(8*n+1);
	bool isSquare = ((8*n+1) == root*root || (8*n+1) == (root+1)*(root+1));
	if (!isSquare || (root+1)%4 != 0) {
		std::cerr << "Number entered is not a hexagonal number\n";
		return 0;
	}

	// Find the next hexagonal number that is also pentagonal
	unsigned long long hex = n, h = (root+1) / 4;
	do {
		h++;
		hex = 2*h*h - h;
	} while (!isPentagonal(hex));

	return hex;
}
