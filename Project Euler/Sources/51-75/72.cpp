#include "stdafx.h"
#include "51-75/72.h"
#include <vector>

// The length of Farey sequence Fn is given by:
//      Fn = 1 + sum(phi(m)) for m from 1 to n
// where phi(m) is Euler's totient function
unsigned long long fareySequenceLength(unsigned long long N) {

    // Generate all of the primes under N
    const unsigned long long sieveSize = N+1;
    bool *primeSieve = new bool[sieveSize]();   // sieve
    for (unsigned long long i = 2; i < sieveSize; i++) primeSieve[i] = true;
    for (unsigned long long i = 2; i < sieveSize; i++)
        if (primeSieve[i])
            for (unsigned long long j = 2; i*j < sieveSize; j++) primeSieve[i*j] = false;
    std::vector<unsigned long long> primes;               // list of primes
    for (unsigned long long i = 0; i < sieveSize; i++)
        if (primeSieve[i]) primes.push_back(i);
    unsigned long long numPrimes = primes.size();

    // Get the totient of every number n by finding n's prime factors
    unsigned long long result = 1;
    for (unsigned long long n = 1; n <= N; n++) {
        unsigned long long phi = 1;
        if (primeSieve[n]) {
            phi = n-1;
        } else {
            unsigned long long x = n;
            unsigned long long primeIndex = 0;
            while (x != 1 && primeIndex < numPrimes) {
                unsigned long long p = primes[primeIndex];
                if (primeSieve[x]) {
                    phi *= (x-1);
                    break;
                }
                if (x % p == 0) {
                    unsigned long long k = 1;
                    while (x % p == 0) {
                        k *= p;
                        x /= p;
                    }
                    phi *= (k / p) * (p-1);
                }
                primeIndex++;
            }
        }
        result += phi;
    }
    result -= 2; // ignore the terms 0 and 1
	return result;
}
