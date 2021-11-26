#include "pch.h"
#include "26-50/33.h"

int findDigitCancellingFractionsDenom() {

	int num = 1, den = 1;

	// Check every fraction: (x1x2) / (y1y2) where (x1, x2, y1, y2) != 0 and x < y
	for (int x1 = 1; x1 <= 9; x1++) {
		for (int x2 = 1; x2 <= 9; x2++) {
			int x = 10*x1+x2;
			for (int y1 = x1; y1 <= 9; y1++) {
				for (int y2 = 1; y2 <= 9; y2++) {
					int y = 10*y1+y2;
					if (x >= y) continue;

					// Consider the new fraction a/b formed from x/y with the common digit removed
					int a = 0, b = 0;
					if (x1 == y1) {
						a = x2;
						b = y2;
					} else if (x1 == y2) {
						a = x2;
						b = y1;
					} else if (x2 == y1) {
						a = x1;
						b = y2;
					} else if (x2 == y2) {
						a = x1;
						b = y1;
					}
					if (a && b && a*y == b*x) {
							std::cout << x << "/" << y << " = " << a << "/" << b << "\n";
							num *= a;
							den *= b;
					}
				}
			}
		}
	}

	// Calculate the denominator of the product of these 4 fractions in lowest terms
	int rem = den % num;
	int gcd = num;
	num = rem;	// use as a temporary value to hold the prev remainder
	while (rem != 0) {
		rem = gcd % rem;
		gcd = num;
		num = rem;
	}

	return den / gcd;
}