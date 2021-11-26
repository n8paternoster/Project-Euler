#include "pch.h"
#include "51-75/64.h"

/*
    Each coefficient of the continued fraction representation of sqrt(n) is generated with these iterative formulas:

        num(i) = a(i)*den(i) - num(i-1)
        den(i) = (n - num(i)^2) / den(i-1)
        a(i) = floor((a0+num(i) / den(i))

        and these initial values:

        num(0) = 0
        den(0) = 1
        a(0) = floor(sqrt(n))

    Every continued fraction represention of sqrt(n) (for n = a non-square, positive, integer) contains a period of length m where the first m-1 coefficients form a palindromic string and a(m) = 2*a(0)
*/
int numOddPeriodContinuedFractionsForSquareRootsUnder(int N) {
    int count = 0;
    for (int n = 2; n <= N; n++) {
        int a0 = (int)sqrt(n);
        if (a0*a0 == n) continue;
        int p = 0, q = 1, a = a0, periodLength = 0;
        while (true) {
            p = a*q - p;
            q = (n - p*p) / q;
            a = (a0 + p) / q;
            periodLength++;
            if (a == a0*2) break;
        }
        if (periodLength%2) count++;
    }
    return count;
}
