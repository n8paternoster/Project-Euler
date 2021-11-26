#include "stdafx.h"
#include "76-100/87.h"
#include <vector>

static std::vector<int> generatePrimes(int N) {
    // Generate all of the primes under N
    const int sieveSize = N;
    bool *primeSieve = new bool[sieveSize]();
    for (int i = 2; i < sieveSize; i++) primeSieve[i] = true;
    for (int i = 2; i < sieveSize; i++)
        if (primeSieve[i])
            for (int j = 2; i*j < sieveSize; j++) primeSieve[i*j] = false;
    std::vector<int> primes;
    for (int i = 0; i < sieveSize; i++)
        if (primeSieve[i]) primes.push_back(i);
    delete[] primeSieve;
    return primes;
}

int numPrimePowerTriples(const int n) {
    // largest prime to consider is sqrt(n) since every prime is at least squared
    std::vector<int> primes = generatePrimes((int)std::sqrt(n));
    std::vector<int> squares, cubes, quarts;
    int maxCubeRoot = (int)std::cbrt(n) + 1;
    int maxFourthRoot = (int)std::pow(n, 1.0/4.0) + 1;
    for (auto p : primes) {
        int s = p*p;
        squares.push_back(s);
        if (p < maxCubeRoot) {
            cubes.push_back(s *= p);
            if (p < maxFourthRoot) quarts.push_back(s*p);
        }
    }
    std::vector<bool> *sums = new std::vector<bool>(n, false);
    int count = 0;
    for (auto q : quarts) {
        for (auto c : cubes) {
            int presum = q+c;
            if (presum >= n) break;
            for (auto s : squares) {
                int sum = presum + s;
                if (sum >= n) break;
                if (!(*sums)[sum]) {
                    (*sums)[sum] = true;
                    count++;
                }
            }
        }
    }
    delete sums;
    return count;
}