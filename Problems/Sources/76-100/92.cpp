#include "pch.h"
#include "76-100/92.h"
#include <vector>

enum class ChainEnd { at1, at89, unknown };

int sumDigitSquares(int n) {
	int result = 0;
	while (n != 0) {
		int digit = n % 10;
		result += digit*digit;
		n /= 10;
	}
	return result;
}

bool arrivesAt89(int n, std::vector<ChainEnd> &chain) {
	if (n < chain.size() && chain[n] == ChainEnd::at89) return true;
	else if (n < chain.size() && chain[n] == ChainEnd::at1) return false;
	int next = sumDigitSquares(n);
	if (arrivesAt89(next, chain)) {
		chain[next] = ChainEnd::at89;
		return true;
	} else {
		chain[next] = ChainEnd::at1;	// all other numbers end at 1
		return false;
	}
}

int numArrivingAt89(int n) {
	// The maximum possible sum is a number with as many digits as n and every digit a 9
	int maxDigitSum = 81*((int)std::log10(n) + 1);

	// Cache the results for every possible sum
	std::vector<ChainEnd> chain = std::vector<ChainEnd>(maxDigitSum+1, ChainEnd::unknown);
	chain[1] = ChainEnd::at1;
	if (chain.size() > 89) chain[89] = ChainEnd::at89;
	int count = 0;
	for (int i = 1; i <= maxDigitSum && i < n; i++) {
		if (arrivesAt89(i, chain)) {
			chain[i] = ChainEnd::at89;
			count++;
		} else chain[i] = ChainEnd::at1;
	}

	// The rest of the numbers can be checked with the previously cached results
	for (int i = maxDigitSum+1; i < n; i++)
		if (chain[sumDigitSquares(i)] == ChainEnd::at89) count++;
	return count;
}
