#include "pch.h"
#include "76-100/77.h"
#include <vector>

static bool isPrime(int n) {
    if (n <= 3) return (n > 1);
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

int sumOfPrimeFactors(const std::vector<int>& primes, int n) {
    int sum = 0;
    for (auto p : primes) {
        if (p > n) break;
        if (n % p == 0) sum += p;
    }
    return sum;
}

/* Using Euler's transform, calculates the number of prime partitions of n using the recursive formula:
*       sopf(n) = sum of n's prime factors
*       pp(n) = number of prime partitions of n
*       pp(n) = (1/n) * [sopf(n) + sum(sopf(k)*pp(n-k)) for k = (1...n-1)]
* 
* Primes below n are cached in the 'primes' vector and pp(n) are cached in the 'primePartitions' vector to speed up the recursion
*/
int numPrimePartitions(const std::vector<int>& primes, std::vector<int>& primePartitions, int n) {
    if (n < 0 || n == 1) return 0;
    if (primePartitions[n]) return primePartitions[n];
    int count = sumOfPrimeFactors(primes, n);
    for (int k = 1; k < n; k++) {
        count += sumOfPrimeFactors(primes, k)*numPrimePartitions(primes, primePartitions, n-k);
    }
    count /= n;
    primePartitions[n] = count;
    return count;
}

int lowestNumberWithNPrimePartitions(int n) {

    // Start with an initial 100 numbers to check (not including 0)
    const int initNum = 100;

    // Get all the primes up to 100
    std::vector<int> primes;
    for (int i = 2; i <= initNum; i++) {
        if (isPrime(i)) primes.push_back(i);
    }

    // Get the prime partition for every number up to 100
    std::vector<int> primePartitions(initNum+1, 0);
    primePartitions[0] = 1;
    numPrimePartitions(primes, primePartitions, initNum);
    int result = 0;
    for (int i = 0; i <= initNum; i++) {
        if (primePartitions[i] > n) {
            result = i;
            break;
        }
    }

    // No result found in the first 100 numbers, check until a result is found
    if (result == 0) {
        int i = initNum;
        do {
            i++;
            if (isPrime(i)) primes.push_back(i);
            primePartitions.push_back(0);
        } while (numPrimePartitions(primes, primePartitions, i) <= n);
        result = i;
    }

    return result;
}
