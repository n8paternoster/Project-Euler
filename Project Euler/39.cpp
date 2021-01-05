#include "stdafx.h"
#include "39.h"

unsigned pythagoreanSumUnderNWithMostSolutions(unsigned N) {

	// p = a + b + c
	// a^2 + b^2 = c^2
	// solving for b in terms of p gives:
	// b = (p*p/2 - pa) / (p - a)

	unsigned maxSolutions = 0, maxP = 0;
	for (unsigned p = 2; p <= N; p += 2) {
		unsigned numSolutions = 0;
		for (unsigned a = 1; a < p / 2; a++) {
			if (((p*p >> 1) - p*a) % (p - a) == 0) {
				unsigned b = ((p*p >> 1) - p*a) / (p - a);
				unsigned c = (unsigned)sqrt(a*a + b*b);
				if (a+b+c == p && a*a + b*b == c*c) numSolutions++;
			}
		}
		if (numSolutions > maxSolutions) {
			maxSolutions = numSolutions;
			maxP = p;
		}
		std::cout << p << " has " << numSolutions << " solutions\n";
	}

	return maxP;
}
