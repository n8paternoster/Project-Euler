#include "pch.h"
#include "51-75/69.h"
#include <vector>

unsigned totientMaxUnderN(unsigned N) {

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

    double maxTotient = 0.0;
    unsigned maxN = 0;
    for (unsigned n = 1; n < N; n++) {
        unsigned phi = 1;
        if (primeSieve[n]) {
            phi = n-1;
        } else {
            unsigned x = n;
            unsigned primeIndex = 0;
            while (x != 1 && primeIndex < primes.size()) {
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
        if (totient > maxTotient) {
            maxTotient = totient;
            maxN = n;
        }
    }

    return maxN;
}
