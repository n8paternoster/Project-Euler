#include "stdafx.h"
#include "9.h"

/*
A Pythagorean triplet is a set of three natural numbers, a < b < c, for which,

a2 + b2 = c2
For example, 32 + 42 = 9 + 16 = 25 = 52.

There exists exactly one Pythagorean triplet for which a + b + c = 1000.
Find the product abc.
*/


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