#include "stdafx.h"
#include "26-50/27.h"

// Simple test that uses the property that primes > 3 are of the form 6k +- 1
static bool isPrime(int n) {
	if (n <= 3) return (n > 1);
	if (n % 2 == 0 || n % 3 == 0) return false;
	for (int i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
		if (n % i == 0 || n % (i + 2) == 0) return false;
	return true;
}

// Counts the number of primes produced from the formula n^2 + a*n + b for consecutive integers of n starting from n = 0
int numPrimesProducedFromQuadratic(int a, int b) {
	int counter = 0;
	for (int n = 0; n < b; n++) {	// when n = b the quadratic becomes b^2 + a*b + b which is not prime (divisible by b)
		if (isPrime(n*n + a*n + b)) counter++;
		else break;
	}
	return counter;
}

/*
	Finds the quadratic formula n^2 + a*n + b for |a| < coeffLimit and |b| <= coeffLimit that produces 
	the most primes for consecutive values of n starting with n = 0 and returns a * b
*/
int coeffProductOfQuadraticProducingMaxNumOfPrimes(int coeffLimit) {
	int maxNumPrimes = 0;
	int aMax = 0;
	int bMax = 0;
	int start = (coeffLimit > 0) ? -coeffLimit : coeffLimit;
	for (int a = (start % 2 == 0) ? start+1 : start+2; a < coeffLimit; a += 2) {	// a must be odd
		for (int b = start; b < coeffLimit; b++) {	// b must be prime since for n = 0, the quadratic = b
			if (!isPrime(b)) continue;
			int numPrimes = numPrimesProducedFromQuadratic(a, b);
			if (numPrimes > maxNumPrimes) {
				aMax = a;
				bMax = b;
				maxNumPrimes = numPrimes;
			}
		}
	}
	std::cout << "a = " << aMax << ", b = " << bMax << "\n" << "Num of primes = " << maxNumPrimes << "\n";
	return aMax * bMax;
}
