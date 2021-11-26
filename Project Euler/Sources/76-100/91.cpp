#include "stdafx.h"
#include "76-100/91.h"

int extendedEuclideanGCD(int a, int b, int &bezout_s, int &bezout_t) {
	// compute the gcd of (a, b) and the Bezout coefficients s and t such that a*s + b*t = gcd(a, b)
	int r_prev = std::abs(a), r = std::abs(b);
	int s_prev = 1, s = 0;
	while (r != 0) {
		int q = r_prev / r;
		int temp = r_prev;
		r_prev = r;
		r = temp - q*r;
		temp = s_prev;
		s_prev = s;
		s = temp - q*s;
	}
	bezout_s = s_prev;
	bezout_t = (b == 0) ? 0 : (r_prev-a*s_prev)/b;
	return r_prev;
}

bool findInitialSolution(int a, int b, int c, int &x0, int &y0, int &d) {
	d = extendedEuclideanGCD(a, b, x0, y0);	// a*bezout_s + b*bezout_t = d
	if (d == 0 || c % d != 0) return false;	// no solutions if c doesn't divide d
	x0 *= c/d;	// x0 = bezout_s * c/d
	y0 *= c/d;	// y0 = bezout_t * c/d
	return true;
}

int countSolutions(int a, int b, int c, int m) {
	if (a == 0 && b == 0) return 0;	// no solutions at origin
	if (a == 0 || b == 0) return m;	// trivial case when the point is on the x or y axis

	// Get an initial solution
	int x0 = 0, y0 = 0, d = 0;
	if (!findInitialSolution(a, b, c, x0, y0, d)) return 0;

	/*
	All other solutions are generated from the initial solution by:
		x = x0 + k*(b/d)
		y = y0 - k*(a/d)
	for any integer k. Solve for the min and max values of k that give positive solutions for (x, y) 
	*/
	a /= d;
	b /= d;
	int minK = -x0/b;
	int maxK = y0/a;
	int count = 0;
	for (int k = minK; k <= maxK; k++) {
		int x = x0 + k*b;
		int y = y0 - k*a;
		if (x > m || y < 0) break;	// while k increases, x increases and y decreases
		if (x >= 0 && y <= m) {
			//std::cout << "k = " << k << ", (" << x << ", " << y << ")\n";
			count++;
		}
	}
	return count;
}

int numRightTriangles(int m) {
	int count = 0;
	for (int a = 0; a <= m; a++) {
		for (int b = 0; b <= m; b++) {
			int c = a*a + b*b;
			count += countSolutions(a, b, c, m);
		}
	}
	return count;
}