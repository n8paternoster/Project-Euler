#include "pch.h"
#include "1-25/9.h"

int pythagoreanTripleSumsTo1000() {
	int a, b, c;

	// Using Euclid's formula for generating pythagorean triples given 2 arbitrary numbers m > n > 0
	for (int n = 1; n < 100; n++) {
		for (int m = n + 1; m < 101; m++) {
			a = (m * m) - (n * n);
			b = 2 * m * n;
			c = (m * m) + (n * n);
			if (a + b + c == 1000) return a * b * c;
		}
	}
	return -1;
}