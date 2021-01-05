#include "stdafx.h"
#include "38.h"
#include <string>

static const unsigned base = 10;	// other bases would require implementing a multiplication algo and different representation for bases > 10 (e.g. string type instead of unsigned)

bool isPandigital(std::string num) {
	if (base < 1) return false;
	if (num.length() != base-1) return false;
	bool *digits = new bool[base-1];
	for (size_t i = 0; i < base-1; i++) digits[i] = false;
	for (auto c : num) {
		size_t index = c - '1';
		if (index > base-1 || digits[index] == true) {
			delete[] digits;
			return false;
		} else digits[index] = true;
	}
	delete[] digits;
	return true;
}

unsigned largestPandigitalProductOfMultiples() {
	if (base > 10 || base < 1) return 0;

	// The integer can be of length 1 to (base-1)/2 (length 1 to 4 in decimal)
	unsigned largest = 0;
	unsigned integer, numMultiples;
	for (unsigned length = 1; length <= (base-1)/2; length++) {

		// Find the valid range of integer values to check (whose multiples will produce (base-1) # digits)
		unsigned maxN = (base-1) / length;	// max num of multiples
		unsigned lowest, highest;
		if ((base-1) % length == 0) {
			lowest = (unsigned)pow(base, length-1);
		} else {
			// Find the largest multiple that can be length (length+1)
			unsigned largestN = maxN;
			while (largestN > 1 && (base-1-length*(largestN-1)) % (length+1) != 0) largestN--;
			if (largestN == 1) continue;
			lowest = (unsigned)(pow(base, length)) / largestN;
		}
		// Find the smallest multiple that can be length (length+1)
		unsigned smallestN = 2;
		while (smallestN <= maxN && (base-1-length*(smallestN-1)) % (length+1) != 0) smallestN++;
		highest = (unsigned)(pow(base, length) - 1) / (smallestN - 1);
		
		// Check all valid integers of this length
		for (unsigned i = highest; i >= lowest; i--) {
			std::string num = "";
			unsigned N = 1;
			while (num.length() < base-1) num += std::to_string(i*N++);
			if (isPandigital(num)) {
				std::cout << "Pandigital found: " << num << ", " << i << "\n";
				unsigned pandigital = std::stoi(num);
				if (pandigital > largest) {
					largest = pandigital;
					integer = i;
					numMultiples = N-1;
				}
				break;	// since all pandigital numbers start with i, no other i of this length will be greater
			}
		}
	}

	for (unsigned i = 1; i <= numMultiples; i++)
		std::cout << integer << " * " << i << " = " << integer*i << "\n";

	return largest;
}
