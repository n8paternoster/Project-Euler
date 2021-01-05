#include "stdafx.h"
#include <unordered_set>

int pandigitalProduct() {
	/* Pandigital products can be in the form:
		x * yyyy = zzzz
		or
		xx * yyy = zzzz
	*/

	std::unordered_set<int> products;
	int sum = 0;

	// Check x * yyyy = zzzz
	for (int x = 2; x < 9; x++) {	// 1 and 9 don't give valid products
		for (int y1 = 1; y1 < 5; y1++) {
			if (y1 == x || x*y1 > 9) continue;
			for (int y2 = 1; y2 < 10; y2++) {
				if (y2 == y1 || y2 == x || x*(10*y1+y2) > 99) continue;
				for (int y3 = 1; y3 < 10; y3++) {
					if (y3 == y2 || y3 == y1 || y3 == x || x*(100*y1+10*y2+y3) > 999) continue;
					for (int y4 = 1; y4 < 10; y4++) {
						if (y4 == y3 || y4 == y2 || y4 == y1 || y4 == x) continue;
						int y = 1000*y1+100*y2+10*y3+y4;
						int prod = x * y;
						if (prod > 9999) continue;

						// Check if the product has unique digits
						bool hasUniqueDigits = true;
						for (int i = 0; i < 4; i++) {
							int digit = (int)(prod/pow(10, i)) % 10;
							if (digit == 0 || digit == x || digit == y1 || digit == y2 || digit == y3 || digit == y4) {
								hasUniqueDigits = false;
								break;
							}
							for (int j = i+1; j < 4; j++)	// check this digit against the others in prod
								if (digit == (int)(prod/pow(10, j)) % 10) {
									hasUniqueDigits = false;
									break;
								}
							if (!hasUniqueDigits) break;
						}
						if (hasUniqueDigits) {
							std::cout << x << " * " << y << " = " << prod << "\n";
							if (products.insert(prod).second) sum += prod;
						}
					}
				}
			}
		}
	}

	// Check xx * yyy = zzzz
	for (int x1 = 1; x1 < 8; x1++) {	// an x with first digit 9 doesn't give a valid product
		for (int x2 = 1; x2 < 10; x2++) {
			if (x2 == x1) continue;
			int x = 10*x1+x2;
			for (int y1 = 1; y1 < 10; y1++) {
				if (y1 == x1 || y1 == x2 || x*y1 > 99) continue;
				for (int y2 = 1; y2 < 10; y2++) {
					if (y2 == y1 || y2 == x1 || y2 == x2 || x*(10*y1+y2) > 999) continue;
					for (int y3 = 1; y3 < 10; y3++) {
						if (y3 == y2 || y3 == y1 || y3 == x2 || y3 == x1) continue;
						int y = 100*y1+10*y2+y3;
						int prod = x * y;
						if (prod > 9999) continue;

						// Check if the product has unique digits
						bool hasUniqueDigits = true;
						for (int i = 0; i < 4; i++) {
							int digit = (int)(prod/pow(10, i)) % 10;
							if (digit == 0 || digit == x1 || digit == x2 || digit == y1 || digit == y2 || digit == y3) {
								hasUniqueDigits = false;
								break;
							}
							for (int j = i+1; j < 4; j++)	// check this digit against the others in prod
								if (digit == (int)(prod/pow(10, j)) % 10) {
									hasUniqueDigits = false;
									break;
								}
							if (!hasUniqueDigits) break;
						}

						if (hasUniqueDigits) {
							std::cout << x << " * " << y << " = " << prod << "\n";
							if (products.insert(prod).second) sum += prod;
						}
					}
				}
			}
		}
	}

	std::cout << "\nUnique products:\n";
	for (auto n : products) std::cout << n << " ";
	std::cout << "\n";
	
	return sum;
}