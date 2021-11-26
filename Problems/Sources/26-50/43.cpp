#include "pch.h"
#include "26-50/43.h"


// Brute force solution generates all 10-digit pandigital numbers
// Optimized by only letting d4 be even and only letting d6 be 0 or 5
long long numSubStringDivisiblePandigitalNumbers() {

	long long sum = 0;
	unsigned amount = 0;
	for (long long a = 0; a < 10; a++) {
		for (long long b = 0; b < 10; b++) {
			if (b == a) continue;
			for (long long c = 0; c < 10; c++) {
				if (c == b || c == a) continue;
				for (long long d = 0; d < 10; d+=2) {	// must de divisible by 2
					if (d == c || d == b || d == a) continue;
					for (long long e = 0; e < 10; e++) {
						if (e == d || e == c || e == b || e == a) continue;
						for (long long f = 0; f < 10; f+=5) {	// must be 0 or 5
							if (f == e || f == d || f == c || f == b || f == a) continue;
							for (long long g = 0; g < 10; g++) {
								if (g == f || g == e || g == d || g == c || g == b || g == a) continue;
								for (long long h = 0; h < 10; h++) {
									if (h == g || h == f || h == e || h == d || h == c || h == b || h == a) continue;
									for (long long i = 0; i < 10; i++) {
										if (i == h || i == g || i == f || i == e || i == d || i == c || i == b || i == a) continue;
										for (long long j = 0; j < 10; j++) {
											if (j == i || j == h || j == g || j == f || j == e || j == d || j == c || j == b || j == a) continue;
											if ((100*h+10*i+j)%17==0 && (100*g+10*h+i)%13==0 && (100*f+10*g+h)%11==0 && (100*e+10*f+g)%7==0 && (c+d+e)%3==0) {
												amount++;
												long long num = 1000000000ll*a + 100000000ll*b + 10000000ll*c + 1000000ll*d + 100000ll*e + 10000ll*f + 1000ll*g + 100ll*h + 10ll*i + j;
												sum += num;
												std::cout << num << "\n";
											}
										}

									}
								}
							}
						}
					}
				}
			}
		}
	}

	std::cout << amount << " matches sum to " << sum << "\n";
	return sum;
}
