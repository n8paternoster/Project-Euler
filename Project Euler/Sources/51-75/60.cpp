#include "stdafx.h"
#include "51-75/60.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>	// std::find, std::sort

using ull = unsigned long long int;

static bool isPrime(ull n) {
	if (n <= 3) return (n > 1);
	if (n % 2 == 0 || n % 3 == 0) return false;
	for (ull i = 5; i*i <= n; i += 6)	// check all numbers up to sqrt(n) of the form 6k +- 1
		if (n % i == 0 || n % (i + 2) == 0) return false;
	return true;
}

bool concatPrimesProducePrimes(ull p1, ull p2) {
	ull cat1 = p1*(ull)pow(10, (ull)(log10(p2)+1)) + p2;
	ull cat2 = p2*(ull)pow(10, (ull)(log10(p1)+1)) + p1;
	return isPrime(cat1) && isPrime(cat2);
}

/* Brute force method generates all sets of primes of length setSize in ascending order by sum and returns the first that satisfies the condition of concatenation among all its members. Generation is done by first generating all pairs in order that display concatenation, then generating all triplets, all quadruplets, etc up to setSize. Scales very poorly, takes ~5secs to solve the given example (setSize = 4) and ~6mins to solve the problem (setSize = 5).
*/
/*
unsigned sumOfSetOfConcatenatingPrimes(unsigned setSize, unsigned maxPrime) {
	if (maxPrime > 1'000'000u) maxPrime = 1'000'000u;
	bool *primeSieve = new bool[maxPrime]();
	std::vector<unsigned> primes;
	for (size_t i = 0; i < maxPrime; i++) primeSieve[i] = true;
	for (size_t i = 2; i < maxPrime; i++) {
		if (primeSieve[i]) {
			primes.push_back(i);
			for (size_t j = 2; i*j < maxPrime; j++) primeSieve[i*j] = false;
		}
	}
	delete[] primeSieve;
	// remove 2 and 5 from consideration
	primes.erase(primes.begin());
	primes.erase(std::find(primes.begin(), primes.end(), 5));

	std::map<unsigned, std::set<std::vector<unsigned>>> n1Sums;

	// Generate all pairs of primes in ascending order of the pair's sum
	unsigned maxSum = primes.back() + primes[primes.size()-2];
	for (unsigned sum = 0; sum <= maxSum; sum += 2) {
		size_t i = 0;
		size_t j = primes.size()-1;
		while (i < j) {
			unsigned p = primes[i], q = primes[j];
			if (p + q == sum) {
				unsigned cat1 = p*(unsigned)pow(10, (unsigned)(log10(q)+1)) + q;
				unsigned cat2 = q*(unsigned)pow(10, (unsigned)(log10(p)+1)) + p;
				if (isPrime(cat1) && isPrime(cat2)) {
					n1Sums[sum].insert(std::vector<unsigned>{p, q});
				}
				i++;
				j--;
			} else if (p + q < sum) i++;
			else j--;
		}
	}

	for (unsigned size = 3; size <= setSize; size++) {
		std::map<unsigned, std::set<std::vector<unsigned>>> nSums;
		maxSum += primes[primes.size()-size];
		for (unsigned nSum = (size%2); nSum <= maxSum; nSum += 2) {
			size_t i = 0;				// iterate over the primes
			auto it = n1Sums.rbegin();	// iterate over the sums of in n1Sums
			while (i < primes.size() && it != n1Sums.rend()) {
				unsigned p = primes[i];
				unsigned n1Sum = it->first;
				if (p + n1Sum == nSum) {
					// Check each n-1 tuple 
					for (auto n1Tuple : n1Sums[n1Sum]) {
						bool isSet = true;
						for (auto q : n1Tuple) {
							if (p == q || !concatPrimesProducePrimes(p, q)) {
								isSet = false;
								break;
							}
						}
						if (isSet) {
							std::vector<unsigned> v(n1Tuple);
							v.push_back(p);
							std::sort(v.begin(), v.end());
							nSums[nSum].insert(v);
							if (size == setSize) {
								std::cout << "Set of size " << setSize << " found:\n";
								unsigned resultSum = 0;
								for (auto u : v) {
									std::cout << u << " ";
									resultSum += u;
								}
								std::cout << "\n";
								return resultSum;
							}
						}
					}
					i++;
					it++;
				} else if (p + n1Sum < nSum) i++;
				else it++;
			}
		}
		n1Sums.clear();
		nSums.swap(n1Sums);
	}

	return 0;
}
*/


unsigned sumOfSetOfConcatenatingPrimes(unsigned setSize, unsigned maxPrime) {
	if (maxPrime > 1'000'000u) maxPrime = 1'000'000u;
	bool *primeSieve = new bool[maxPrime]();
	std::vector<unsigned> primes;
	for (size_t i = 0; i < maxPrime; i++) primeSieve[i] = true;
	for (size_t i = 2; i < maxPrime; i++) {
		if (primeSieve[i]) {
			primes.push_back(i);
			for (size_t j = 2; i*j < maxPrime; j++) primeSieve[i*j] = false;
		}
	}
	delete[] primeSieve;

	// remove 2 and 5 from consideration
	primes.erase(primes.begin());	
	primes.erase(std::find(primes.begin(), primes.end(), 5));

	std::map<unsigned, std::set<unsigned>> concatPrimes;	// map each prime to a set of other primes that can concatenate with it
	std::set<std::vector<unsigned>> results;
	unsigned returnVal = UINT_MAX;
	for (size_t i = 0; i < primes.size(); i++) {

		unsigned p = primes[i];
		for (size_t j = 0; j < i; j++) {

			// check if q can concatenate with p to produce 2 other primes
			unsigned q = primes[j];
			if (!concatPrimesProducePrimes(p, q)) continue;

			// check if q can concatenate with the set of concatenatable primes in p
			bool isSetMember = true;
			for (auto pSetMember : concatPrimes[p]) {
				if (!concatPrimesProducePrimes(q, pSetMember)) {
					isSetMember = false;
					break;
				}
			}
			if (isSetMember) concatPrimes[p].insert(q);

			// check if p can concatenate with the set of concatenatable primes in q
			isSetMember = true;
			for (auto qSetMember : concatPrimes[q]) {
				if (!concatPrimesProducePrimes(p, qSetMember)) {
					isSetMember = false;
					break;
				}
			}
			if (isSetMember) concatPrimes[q].insert(p);

			// result found
			if (concatPrimes[p].size() == setSize-1) {
				unsigned sum = p;
				std::vector<unsigned> v = {p};
				for (auto setPrime : concatPrimes[p]) {
					v.push_back(setPrime);
					sum += setPrime;
				}
				std::sort(v.begin(), v.end());
				if (results.insert(v).second) {	// unique result found
					if (sum < returnVal) returnVal = sum;
					std::cout << "Set of size " << setSize << " found: ";
					for (auto p : v) std::cout << p << " ";
					std::cout << "= " << sum << "\n";
				}
			}
		}
	}

	if (returnVal == UINT_MAX) returnVal = 0;	// none found
	return returnVal;
}
