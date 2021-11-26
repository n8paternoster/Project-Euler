#include "stdafx.h"
#include "51-75/57.h"
#include "bigNum.h"

/*
    Brute force solution generates all convergents as a numerator and denominator and compares their number of digits. 
    The ith convergent is formed from the relation:
        num(i) = num(i-1) + 2*den(i-1)
        den(i) = num(i-1) + den(i-1)
*/
unsigned numConvergentsWithLargerNumerator(unsigned numExpansions) {
    bigNum num("3"), den("2");
    bigNum p, q, two("2");
    unsigned count = 0;
    for (unsigned e = 2; e <= numExpansions; e++) {
        p = num + two*den;
        q = num + den;
        num = p;
        den = q;
        if (num.numDigits() > den.numDigits()) count++;
    }
    return count;
}
