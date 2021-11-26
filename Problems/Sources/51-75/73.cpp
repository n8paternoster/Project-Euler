#include "pch.h"
#include "51-75/73.h"
#include <numeric>

// Brute-force generates all of the terms of the Farey sequence in order and checks the index each given fraction is located at
unsigned fareySequenceDistanceBetween(unsigned n1, unsigned d1, unsigned n2, unsigned d2, unsigned n) {
    if (n1 > d1 || n2 > d2 || d1 > n || d2 > n) return 0;

    // Reduce the input
    while (std::gcd(n1, d1) != 1) {
        auto d = std::gcd(n1, d1);
        n1 /= d;
        d1 /= d;
    }
    while (std::gcd(n2, d2) != 1) {
        auto d = std::gcd(n2, d2);
        n2 /= d;
        d2 /= d;
    }

    // first term a/b, second term c/d
    unsigned a = 0, b = 1;
    unsigned c = 1, d = n;
    unsigned counter = 0;
    while (c <= n) {
        if (a == n1 && b == d1) counter = 0;
        else if (a == n2 && b == d2) break;
        unsigned k = (n + b) / d;
        unsigned tempA = a, tempB = b;
        a = c;
        b = d;
        c = k*c - tempA;
        d = k*d - tempB;
        counter++;
    }

    return counter-1;
}
