#include "stdafx.h"
#include "26-50/44.h"

bool isPentagonal(unsigned x) {
	unsigned root = (unsigned)sqrt(x*24+1);
	bool isSquare = (x*24+1) == root*root || (x*24+1) == (root+1)*(root+1);	// for floating-point error
	return (isSquare && (root+1)%6 == 0);
}

/* Iterates through the possible differences two pentagonal numbers x and y could have (diff). If x + diff is a pentagonal number, then let y = x + diff. If this y + x is also a pentagonal number, we found our answer since all other possible differences lower than diff have been checked. */
unsigned differenceOfMinimalPentagonNumbers() {

	auto P = [](unsigned n) -> unsigned {
		return n*(3*n-1)/2;
	};

	unsigned j = 1, x = 1;
	while (true) {
		unsigned k = 1, diff = 1;
		while (diff + x > P(k+1)) {	// check until the difference between adjacent pentagonal numbers is > diff
			if (isPentagonal(diff+x) && isPentagonal(diff+x+x) && k != j) {
				std::cout << "x = " << x << ", y = " << diff+x << ", x-y = " << diff << ", x+y = " << diff+x+x << "\n";
				return diff;
			}
			k++;
			diff += 3*k - 2;
		}
		j++;
		x += 3*j - 2;
	}

	return 0;
}
