#include "stdafx.h"
#include "51-75/71.h"
#include <numeric>

std::pair<unsigned, unsigned> fareySequenceLeftNeighbor(unsigned num, unsigned den, unsigned n) {
    if (num > n || den > n || num > den) return {0, 0};

    // Reduce the input
    while (std::gcd(num, den) != 1) {
        auto d = std::gcd(num, den);
        num /= d;
        den /= d;
    }

    unsigned bestNum = 0, bestDen = 1;
    unsigned currDen = n, minDen = 1;
    while (currDen > minDen) {
        unsigned currNum = (num*currDen - 1) / den;
        if (bestNum*currDen < currNum*bestDen) {
            bestNum = currNum;
            bestDen = currDen;
            minDen = currDen / (num*currDen - den*currNum);
        }
        currDen--;
    }

    // Reduce the result
    while (std::gcd(bestNum, bestDen) != 1) {
        auto d = std::gcd(bestNum, bestDen);
        bestNum /= d;
        bestDen /= d;
    }
    return {bestNum, bestDen};
}
