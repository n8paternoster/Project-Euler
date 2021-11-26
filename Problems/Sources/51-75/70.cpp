#include "pch.h"
#include "51-75/70.h"
#include <vector>

// Checks if one number is a permutation of another
bool isPermutationOf(unsigned p, unsigned q) {
    unsigned pDigits = (unsigned)log10(p)+1;
    unsigned qDigits = (unsigned)log10(q)+1;
    if (pDigits != qDigits) return false;
    int digits[10] = {};
    while (p != 0) {
        digits[p%10]++;
        p /= 10;
    }
    while (q != 0) {
        digits[q%10]--;
        q /= 10;
    }
    for (auto d : digits)
        if (d) return false;
    return true;
}

unsigned minTotientPermutationUnderN(unsigned N) {

    // Generate all of the primes under N
    const unsigned sieveSize = N;
    bool *primeSieve = new bool[sieveSize]();   // sieve
    for (unsigned i = 2; i < sieveSize; i++) primeSieve[i] = true;
    for (unsigned i = 2; i < sieveSize; i++)
        if (primeSieve[i])
            for (unsigned j = 2; i*j < sieveSize; j++) primeSieve[i*j] = false;
    std::vector<unsigned> primes;               // list of primes
    for (unsigned i = 0; i < sieveSize; i++)
        if (primeSieve[i]) primes.push_back(i);
    unsigned numPrimes = primes.size();

    // Get the totient of every number n by finding n's prime factors
    double minTotient = HUGE_VAL;
    unsigned minN = 0;
    for (unsigned n = 2; n < N; n++) {
        unsigned phi = 1;
        if (primeSieve[n]) {
            phi = n-1;
        } else {
            unsigned x = n;
            unsigned primeIndex = 0;
            while (x != 1 && primeIndex < numPrimes) {
                unsigned p = primes[primeIndex];
                if (primeSieve[x]) {
                    phi *= (x-1);
                    break;
                }
                if (x % p == 0) {
                    unsigned k = 1;
                    while (x % p == 0) {
                        k *= p;
                        x /= p;
                    }
                    phi *= (k / p) * (p-1);
                }
                primeIndex++;
            }
        }
        double totient = static_cast<double>(n) / static_cast<double>(phi);
        if (totient < minTotient && isPermutationOf(n, phi)) {
            minTotient = totient;
            minN = n;
        }
    }
    std::cout << "Minimum totient: " << minTotient << "\n";
    return minN;
}
