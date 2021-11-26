#pragma once

/*
	The sum of the primes below 10 is 2 + 3 + 5 + 7 = 17.

	Find the sum of all the primes below two million.
*/

uint64_t summationOfPrimesBelowN(uint64_t n);
inline auto p10() {
	return summationOfPrimesBelowN(2'000'000);
}