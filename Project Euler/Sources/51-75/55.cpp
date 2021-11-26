#include "stdafx.h"
#include "51-75/55.h"
#include <vector>

void incrementVector(std::vector<unsigned>& v) {
    unsigned carry = 1;
    auto it = v.begin();
    while (carry && it != v.end()) {
        *it += carry;
        if (*it > 9) {
            *it %= 10;
            carry = 1;
        } else carry = 0;
        it++;
    }
    if (carry) v.push_back(carry);
}

bool isPalindrome(const std::vector<unsigned>& n) {
    auto it = n.begin();
    auto rit = n.rbegin();
    for (; it != n.end(); ++it, ++rit)
        if (*it != *rit) return false;
    return true;
}

std::vector<unsigned> reverseAndAdd(const std::vector<unsigned>& n) {
    std::vector<unsigned> result(n.size(), 0);
    auto it = n.begin();
    auto rit = n.rbegin();
    unsigned carry = 0;
    for (unsigned i = 0; it != n.end(); ++it, ++rit, ++i) {
        result[i] = *it + *rit + carry;
        if (result[i] > 9) {
            result[i] %= 10;
            carry = 1;
        } else carry = 0;
    }
    if (carry) result.push_back(carry);
    return result;
}


// Possible to speed up by using unsigned long long instead of vector<unsigned> but I wanted the ability to check numbers above 10000
unsigned numLychrelNumbersBelowN(unsigned N, unsigned numIterations) {
    std::vector<unsigned> seed = {0};
    unsigned count = 0;
    for (unsigned i = 1; i < N; i++) {
        incrementVector(seed);
        std::vector<unsigned> n = seed;
        unsigned iteration = 0;
        bool isLychrel = true;
        do {
            n = reverseAndAdd(n);
            iteration++;
            if (isPalindrome(n)) isLychrel = false;
        } while (isLychrel && iteration <= numIterations);
        if (isLychrel) count++;
    }
    return count;
}
