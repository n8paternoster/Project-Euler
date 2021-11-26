#pragma once

/*
	The prime factors of 13195 are 5, 7, 13 and 29.

	What is the largest prime factor of the number 600851475143 ?
*/

uint64_t largestPrimeFactorOf(uint64_t);
inline auto p3() {
	return largestPrimeFactorOf(600'851'475'143ull);
}