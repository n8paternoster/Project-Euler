#include "stdafx.h"
#include "26-50/41.h"

static bool isPrime(int n) {
	if (n <= 3) return (n > 1);
	if (n % 2 == 0 || n % 3 == 0) return false;
	for (int i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
		if (n % i == 0 || n % (i + 2) == 0) return false;
	return true;
}

// Brute force check generates all pandigital numbers in descending order starting from 7654321 (numbers of length 8 or 9 are all divisible by 3)
int largestPandigitalPrime() {
	for (int n = 7; n > 0; n--) {	// n length pandigital numbers
		int a = (n > 6) ? n : 0;
		do {
			int b = (n > 5) ? n : 0;
			do {
				if (b != 0 && b == a) { b--; continue; }
				int c = (n > 4) ? n : 0;
				do {
					if (c != 0 && (c == b || c == a)) { c--; continue; }
					int d = (n > 3) ? n : 0;
					do {
						if (d != 0 && (d == c || d == b || d == a)) { d--; continue; }
						int e = (n > 2) ? n : 0;
						do {
							if (e != 0 && (e == d || e == c || e == b || e == a)) { e--; continue; }
							int f = (n > 1) ? n : 0;
							do {
								if (f != 0 && (f == e || f == d || f == c || f == b || f == a)) { f--; continue; }
								for (int g = n; g > 0; g--) {
									if (g == f || g == e || g == d || g == c || g == b || g == a) { g--; continue; }
									int num = 1000000*a+100000*b+10000*c+1000*d+100*e+10*f+g;
									if (isPrime(num)) return num;
								}
							f--; } while (f > 0);
						e--; } while (e > 0);
					d--; } while (d > 0);
				c--; } while (c > 0);
			b--; } while (b > 0);
		a--; } while (a > 0);
	}
	return 0;
}
