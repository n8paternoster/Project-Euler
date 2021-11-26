#include "pch.h"
#include "51-75/66.h"
#include "bigNum.h"

/*
	A pell equation is any diophantine equation in the form:

	x^2 - Ny^2 = 1
	
	where N is a positive nonsquare integer and integer solutions are sought for x and y. The fundamental solution to this equation is the smallest integer solution for x and y (excluding the trivial solution of {1, 0}). This solution is found using the Chakravala method which considers the similar equation:

	x^2 - Ny^2 = k

	An initial triple (x, y, k) satisfying the equation for a given N is 'composed' with the trivial triple (m, 1, m^2-N) and scaled down by k^2 to get a new triple:
	
	{ (xm + Ny)/|k| , (x + ym)/|k| , (m^2 - N)/k }
	
	This process is repeated until k = 1.

	Sources:
		https://en.wikipedia.org/wiki/Chakravala_method
		http://www.ms.uky.edu/~sohum/AAK/pdf%20files/chakravala.pdf
		http://bmvamg.org.in/BMVpdf/ProjectSMD.pdf
*/
std::pair<bigNum, bigNum> fundamentalSolutionToPell(int N) {

	// Find an initial triple by setting y to 1 and solving for the smallest k
	int root = (int)std::floor(std::sqrt(N));
	if (root*root == N) return {1, 0};
	int k;
	bigNum x;
	if (std::abs(root*root - N) < std::abs((root+1)*(root+1) - N)) {
		k = root*root - N;
		x = root;
	} else {
		k = (root+1)*(root+1) - N;
		x = root+1;
	}
	int absK = std::abs(k);
	bigNum y = 1;

	// Generate a new triple each iteration until k = 1
	while (k != 1) {

		// If k = -1 or +-2, +-4 we can make use of Brahmagupta's method of composition to arrive directly at a solution for k = 1
		// These all can be derived from Brahmagupta's identity that allows the composition of two triples into a third:
		//		(x1, y1, k1) composed with (x2, y2, k2) yields (x1x2 + Ny1y2, x1y2 + x2y1, k1k2)
		if (k == -1) {
			y *= 2*x;
			x = 2*x*x + 1;
			k = 1;
			break;
		} else if (k == 2 || k == -2) {
			y *= x;
			x = x*x + (k > 0 ? -1 : 1);
			k = 1;
			break;
		} else if (k == 4) {
			if (x%2 == 0) {
				y *= x/2;
				x = (x*x)/2 - 1;
			} else {
				y *= (x*x-1)/2;
				x *= (x*x-3)/2;
			}
			k = 1;
			break;
		} else if (k == -4) {
			if (x%2 == 0) {
				y *= x/2;
				x = (x*x)/2 + 1;
			} else {
				bigNum z = x*x;
				y = (x*y*(z+1)*(z+3))/2;
				x = (z+2)*((z+1)*(z+3)-2)/2;
			}
			k = 1;
			break;
		}

		// Choose m such that:
		//		(1) (x + y*m)/k is an integer 
		//		(2) |m*m - N| is minimal

		// (1) solve (x + y*m) congruent to 0 (mod k) for m using the extended euclidean algorithm 
		int a = (int)(bigNum::euclideanDivide(x, absK)*absK - x) + absK;
		int b = (int)(y % absK);
		int oldA = 0, oldB = absK;
		while (b != 1) {
			if (b == 0) {
				std::cout << "Error\n";
				return {0, 0};
			}
			int t = oldB/b;
			int newA = oldA - t*a, newB = oldB - t*b;
			while (newA < 0) newA += absK;
			while (newB < 0) newB += absK;
			newA %= absK; newB %= absK;
			oldA = a; oldB = b;
			a = newA; b = newB;
		}
		int m = a;

		// (2) the minimal m will be either the m directly below root(N) or directly above
		while (m < root) m += absK;
		if (std::abs(m*m-N) > std::abs((m-absK)*(m-absK)-N)) m -= absK;

		// Update triple using this m
		bigNum prevX = x;
		x = (prevX*m + N*y) / absK;
		y = (prevX + y*m) / absK;
		k = (m*m - N) / k;
		absK = std::abs(k);
	}

	return {x, y};
}


int pellEqCoeffWithLargestMinimalX(int maxN) {
	bigNum maxX = 0;
	int result = 0;
	for (int n = 1; n <= maxN; n++) {
		std::pair<bigNum, bigNum> solution = fundamentalSolutionToPell(n);
		bigNum &x = solution.first, &y = solution.second;
		std::cout << "N = " << n << ": (x, y) = (";
		x.print();
		std::cout << ", ";
		y.print();
		std::cout << ")\n";
		if (x > maxX) {
			maxX = x;
			result = n;
		}
	}
	return result;
}
