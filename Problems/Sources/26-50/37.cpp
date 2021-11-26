#include "pch.h"
#include "26-50/37.h"

static bool isPrime(unsigned n) {
	if (n <= 3) return (n > 1);
	if (n % 2 == 0 || n % 3 == 0) return false;
	for (unsigned i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
		if (n % i == 0 || n % (i + 2) == 0) return false;
	return true;
}

unsigned sumOfTruncatablePrimes() {
	unsigned sum = 0;
	for (unsigned n = 11; n < 1000000; n += 2) {
		if (isPrime(n)) {
			bool isTruncatable = true;
			unsigned r = n / 10;
			while (r != 0 && isTruncatable) {
				if (!isPrime(r)) isTruncatable = false;
				r /= 10;
			}
			unsigned length = (unsigned)log10(n);
			unsigned l = n - (unsigned)(n/pow(10,length))*pow(10,length);
			while (l != 0 && isTruncatable) {
				if (!isPrime(l)) isTruncatable = false;
				length--;
				l -= (unsigned)(l/pow(10, length))*pow(10, length);
			}
			if (isTruncatable) {
				std::cout << n << "\n";
				sum += n;
			}
		}
	}
	return sum;
}