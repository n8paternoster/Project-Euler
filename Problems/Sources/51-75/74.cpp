#include "pch.h"
#include "51-75/74.h"
#include <vector>
#include <algorithm>

const unsigned fact[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880};    // n! for n = [0, 9]

inline unsigned digitFactorial(unsigned n) {
    unsigned result = 0;
    while (n != 0) {
        result += fact[n%10];
        n /= 10;
    }
    return result;
}

/* saves the chain length of every number (less than the max starting number) calculated so far */
unsigned numDigitFactorialChains(unsigned length, unsigned maxStartingNumber) {
    unsigned *chainLengths = new unsigned[maxStartingNumber]();
    unsigned count = 0;
    for (unsigned start = 1; start < maxStartingNumber; start++) {

        // Check if this number has been seen before
        if (chainLengths[start]) {
            if (chainLengths[start] == length) count++;
            continue;
        }

        // Generate the chain of numbers starting at 'start'
        std::vector<unsigned> chain;
        unsigned n = start;
        unsigned startLength = 0;
        while (std::find(chain.begin(), chain.end(), n) == chain.end()) {
            chain.push_back(n);
            n = digitFactorial(n);
            if (n < maxStartingNumber && chainLengths[n]) {
                startLength = chainLengths[n];
                break;
            }
        }
        startLength += chain.size();

        // Update chain lengths for every number in this chain
        bool inCycle = false;
        unsigned depth = startLength;
        for (auto c : chain) {
            if (c < maxStartingNumber) chainLengths[c] = depth;
            inCycle |= (c == n);    // numbers within the chain's cycle have the same chain length
            if (!inCycle) depth--;  // otherwise each successive number has chain length: (depth - 1)
        }
        if (startLength == length) count++;
    }
    delete[] chainLengths;
    return count;
}

/* original brute force, does not cache chain lengths, very slow >60s */
//unsigned numDigitFactorialChains(unsigned chainLength, unsigned maxStartingNumber) {
//    unsigned count = 0;
//    for (unsigned start = 1; start < maxStartingNumber; start++) {
//        unsigned n = start;
//        std::unordered_set<unsigned> chainNumbers;
//        while (chainNumbers.insert(n).second) {
//            n = digitFactorial(n);
//        }
//        if (chainNumbers.size() == chainLength) count++;
//    }
//    return count;
//}
