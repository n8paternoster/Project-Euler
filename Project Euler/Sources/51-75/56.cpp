#include "stdafx.h"
#include "51-75/56.h"
#include "bigNum.h"

// Brute force solution begins checking with 100^100. Skips any base divisible by 10 and stops checking terms when there aren't enough digits left to sum to greater than maxSum even if all 9's
unsigned maximumDigitSum(unsigned abLimit) {
    unsigned base = abLimit;
    unsigned maxSum = 0, maxBase = 0, maxE = 0;
    while (base > 0) {
        if (base % 10 == 0) {
            base--;
            continue;
        }
        bigNum b = (std::to_string(base));
        bigNum n = b ^ abLimit;
        if (9 * n.numDigits() < maxSum) break;
        unsigned e = abLimit;
        while (n > b) {
            if (9 * n.numDigits() < maxSum) break;
            unsigned sum = 0;
            for (auto d = 0; d < n.numDigits(); d++) {
                if (sum + 9 * (n.numDigits() - d) < maxSum) break;
                sum += n[d];
            }
            if (sum > maxSum) {
                maxSum = sum;
                maxBase = base;
                maxE = e;
            }
            n = n / b;
            e--;
        }
        base--;
    }
    std::cout << maxBase << " ^ " << maxE << " = " << maxSum << " (digit sum)\n";
    return maxSum;
}
