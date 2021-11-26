#include "stdafx.h"
#include "26-50/29.h"

// Simple test that uses the property that primes > 3 are of the form 6k +- 1
static bool isPrime(unsigned n) {
	if (n <= 3) return (n > 1);
	if (n % 2 == 0 || n % 3 == 0) return false;
	for (unsigned i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
		if (n % i == 0 || n % (i + 2) == 0) return false;
	return true;
}

// Checks if n can be expressed as integers x^y
bool isPower(unsigned n) {
	for (unsigned x = 2; x*x <= n; x++) {
		unsigned y = 2;
		unsigned p = (unsigned)pow(x, y);
		while (p <= n) {
			if (p == n) return true;
			y++;
			p = (unsigned)pow(x, y);
		}
	}
	return false;
}

// Counts the number of distinct terms generated from a^b where 2 <= a <= n and 2 <= b <= n
unsigned numDistinctPowersUnderN(unsigned n) {
	if (n > sqrt(UINT_MAX)) throw;
	if (n < 2) return 0;

	unsigned numDistinctPowers = (n-1)*(n-1);

	// Subtract the number of terms eliminated by repitition
	for (unsigned a = 2; a*a <= n; a++) {
		if (isPower(a)) continue;					// check if a can be written as x^y
		unsigned aNum = (unsigned)(log(n)/log(a));	// the number of bases under n that are a power of a
		for (unsigned b = 4; b <= n*(aNum-1); b++) {
			if (isPrime(b)) continue;
			unsigned numTerms = 0;
			for (unsigned d = 1; d <= aNum && d <= b/2; d++)	// check divisors of b
				if (b%d == 0 && b/d <= n) numTerms++;
			if (numTerms >= 2) numDistinctPowers -= numTerms-1;
		}
	}

	return numDistinctPowers;
}