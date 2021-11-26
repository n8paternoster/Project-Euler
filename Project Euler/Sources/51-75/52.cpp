#include "stdafx.h"
#include "51-75/52.h"

bool IsPermutation(unsigned a, unsigned b) {
	int digits[10] = {0};
	while (a != 0) {
		digits[a%10]++;
		a /= 10;
	}
	while (b != 0) {
		digits[b%10]--;
		b /= 10;
	}
	for (auto d : digits)
		if (d != 0) return false;

	return true;
}

unsigned smallestPermutatedNMultiple(unsigned n) {
	if (n > 7 || n < 2) return 0;
	for (unsigned i = 1; i < 1'000'000; i++) {
		if (i*n > (unsigned)pow(10, (unsigned)log10(i)+1)) i = (unsigned)pow(10,(unsigned)log10(i)+1);
		bool isSolution = true;
		for (unsigned j = 2; j <= n; j++)
			if (!(isSolution = IsPermutation(i, j*i))) break;
		if (isSolution) return i;
	}
	return 0;
}
