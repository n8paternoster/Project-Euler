#include "pch.h"
#include "76-100/95.h"
#include <vector>
#include <algorithm>    // std::min_element

enum class color { white, grey, black };    // unvisited, visiting, visited

static std::vector<int> generatePrimes(int N) {
    bool* primeSieve = new bool[N]();
    for (int i = 2; i < N; i++) primeSieve[i] = true;
    for (int i = 2; i * i < N; i++)
        if (primeSieve[i])
            for (int j = i * i; j < N; j += i) primeSieve[j] = false;
    std::vector<int> primes;
    for (int i = 0; i < N; i++)
        if (primeSieve[i]) primes.push_back(i);
    delete[] primeSieve;
    return primes;
}

void getAliquotSums(int *aliquot, int N, const std::vector<int> &primes) {
    /* Calculate sigma(n) (sum of divisors of n) for all n up to N:
    *       sigma(n) = product( (p^(a+1)-1)/(p-1) )
    *   for every distinct prime factor (p) of n where a is the multiplicity of p
    *   and subtract out n to get the aliquot sum (sum of proper divisors; all divisors except the number itself)
    */
    for (int n = 2; n < N; n++) {
        int num = n;	// make a copy of n to reduce 
        int sigma = 1;
        // Find the distinct prime factors of n up to sqrt(n)
        for (auto p : primes) {
            if (p * p > n) break;
            if (n % p != 0) continue;
            int m = p;			// m = p^(a+1)
            while (n % m == 0)	// get the multiplicity of p
                m *= p;
            num /= (m / p);		// reduce n with each prime factor
            sigma *= (m - 1) / (p - 1);
        }
        // If n is prime or has a prime factor > sqrt(n),
        // it is a distinct prime factor with multiplicity of 1
        if (num > 1) sigma *= (num + 1);
        aliquot[n] = sigma - n;
    }
}

bool dfs(int curr, int *vertices, int N, std::vector<color> &visited, std::vector<int> &cycle) {
    // Return true if the curr vertex is part of a cycle, false if not
    
    // vertex is out of range, already visited, or a dead-end
    if (curr >= N || visited[curr] == color::black) return false;
    if (vertices[curr] == 1) {  // dead-end
        visited[curr] = color::black;
        return false;
    }

    // vertex was seen before, cycle detected
    if (visited[curr] == color::grey) {
        cycle.push_back(curr);
        visited[curr] = color::black;
        return true;
    }

    // vertex is unvisited, mark as visiting and check the vertex it points to
    visited[curr] = color::grey;
    if (dfs(vertices[curr], vertices, N, visited, cycle) && visited[curr] == color::grey) {
        // the vertex curr points to is part of a cycle and curr belongs to that cycle
        cycle.push_back(curr);
        visited[curr] = color::black;
        return true;
    } else {
        // vertex is not part of a cycle, mark as visited
        visited[curr] = color::black;
        return false;
    }
}

int longestAmicableChain(int N) {

    // Generate all of the primes under sqrt(N)
    std::vector<int> primes = generatePrimes(static_cast<int>(std::sqrt(N)));

    // Get the aliquot sum for every number up to N
    int *aliquot = new int[N + 1]();
    getAliquotSums(aliquot, N, primes);

    /*
    * Every number is a vertex in a directed graph that points to 1 other vertex
    *  Perform a depth-first search to detect a cycle and mark all vertices as visited
    *  so they are only visited once
    */
    std::vector<color> visited = std::vector<color>(N+1, color::white); // initialize all vertices as unvisited
    std::vector<int> chain, longestChain;
    for (int n = 2; n < N+1; n++) {
        if (aliquot[n] == 1 || visited[n] == color::black) continue;
        dfs(n, aliquot, N+1, visited, chain);
        if (chain.size() > longestChain.size()) longestChain = chain;
        chain.clear();
    }
    delete[] aliquot;
    if (longestChain.empty()) return 0;

    // Print chain
    std::cout << "Chain of length " << longestChain.size() << " found:\n";
    for (auto it = longestChain.rbegin(); it != longestChain.rend(); ++it) std::cout << *it << "->";
    std::cout << longestChain.back() << "\n";
    return *std::min_element(longestChain.begin(), longestChain.end());
}