#include "stdafx.h"
#include "26-50/28.h"

unsigned sumOfNSpiralDiagonals(unsigned n) {
	if (n <= 1) return n;
	unsigned sum = n%2;	// when n is odd, the diagonals include an additional number in the center (1)
	for (unsigned i = 0; i < (n-n%2)/2; i++)
		sum += 4*(n-(2*i))*(n-(2*i)) - 6*(n-(2*i+1));
	return sum;
}