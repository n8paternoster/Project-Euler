#include "stdafx.h"
#include "76-100/80.h"
#include "bigNum.h"

bigNum squareRoot(int n, int p) {

    bigNum::setPrecision(p);
    
    // Rewrite n as (a * 10^2e) where 1 <= a < 100
    int e = (int)log10(n) + 1;
    e -= (e % 2 == 0) ? 2 : 1;  // exponent must be even
    e /= 2;
    bigNum a = (float)n / powf(10, 2*e);
    
    // Get an initial estimate for the square root of n
    // A piecewise linear estimate for the function y = x^2 gives:
    // sqrt(n) ~= { (0.28a + 0.89) * 10^n    if a < 10
    //            { (0.089a + 2.8) * 10^n    if a >= 10
    bigNum x = (a < 10) ? (0.28*a + 0.89) : (0.089*a + 2.8);
    x *= pow(10, e);

    // Use the Babylonian method to calculate the square root to p precision
    // Each iteration refines the previous estimate by taking the average of the previous estimate and the (previous estimate / n); one is an overestimate and the other an underestimate of the true root
    // One is an overestimate and the other an underestimate
    bigNum prevX = 0;
    bigNum s = n;

    const int maxIter = 20;
    int counter = 0;
    while (x != prevX && counter++ < maxIter) {
        bigNum temp = x;
        x = (x + s/x) / 2;
        prevX = temp;
    }
    return x;
}

int digitSumFirstNSquareRoots(int n, int precision) {
    
    int result = 0;
    for (int i = 2; i <= n; i++) {
        int r = (int)sqrt(i);
        if (i == r*r || i == (r+1)*(r+1)) continue;

        // Calculate the square root
        bigNum root = squareRoot(i, precision);
        std::cout << "sqrt(" << i << ") =\t" << root << "\n";

        // Sum the digits
        int digitSum = 0;
        for (int d = 0; d < root.numDigits(); d++)
            digitSum += root[d];
        result += digitSum;
    }

    return result;
}
