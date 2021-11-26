#include "pch.h"
#include "76-100/95.h"
#include <vector>
#include <algorithm>    // std::min_element

enum class color { white, grey, black };    // unvisited, visiting, visited

static std::vector<int> generatePrimes(int n) {
    bool *primeSieve = new bool[n]();
    for (int i = 2; i < n; i++) primeSieve[i] = true;
    for (int i = 2; i < n; i++)
        if (primeSieve[i])
            for (int j = 2; i*j < n; j++) primeSieve[i*j] = false;
    std::vector<int> primes;
    for (int i = 0; i < n; i++)
        if (primeSieve[i]) primes.push_back(i);
    delete[] primeSieve;
    return primes;
}

bool dfs(int curr, int *vertices, int n, std::vector<color> &visited, std::vector<int> &cycle) {
    // Return true if the curr vertex is part of a cycle, false if not
    
    // vertex is out of range, already visited, or a dead-end
    if (curr >= n || visited[curr] == color::black) return false;
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
    if (dfs(vertices[curr], vertices, n, visited, cycle) && visited[curr] == color::grey) {
        // the vertex curr points to is part of a cycle and curr belongs to that cycle
        cycle.push_back(curr);
        visited[curr] = color::black;
        return true;
    }

    // vertex is not part of a cycle, mark as visited
    visited[curr] = color::black;
    return false;
}

int longestAmicableChain(int n) {

    // Generate all of the primes under sqrt(n)
    std::vector<int> primes = generatePrimes(static_cast<int>(std::sqrt(n)));

    /* Calculate sigma(x) (sum of divisors of x) for all x up to n:
    *       sigma(x) = product( (p^(a+1)-1)/(p-1) )
    *   for every distinct prime factor (p) of x where a is the multiplicity of p
    *   and subtract out x to get the aliquot sum (sum of proper divisors; all divisors except the number itself)
    */
    int *sigma = new int[n+1]();
    for (int x = 2; x < n; x++) {
        int num = x;
        int sum = 1;
        // Find the distinct prime factors of x up to sqrt(x)
        for (auto p : primes) {
            if (p*p > x) break;
            if (x % p != 0) continue;
            int m = p;      // m will be p^(a+1)
            while (x % m == 0) m *= p;
            num /= (m/p);   // reduce x at each step
            sum *= (m-1) / (p-1);
        }
        // If x is prime or has a prime factor > sqrt(x) it is a distinct prime factor with multiplicity of 1
        if (num > 1) sum *= (num+1);    
        sigma[x] = sum - x;     // aliquot sum
    }

    /* Every number is a vertex in a directed graph that points to 1 other vertex
    *  Perform a depth-first search to detect a cycle and mark all vertices as visited so they are only visited once
    */
    std::vector<color> visited = std::vector<color>(n+1, color::white); // initialize all vertices as unvisited
    std::vector<int> chain, longestChain;
    for (int x = 2; x < n+1; x++) {
        if (sigma[x] == 1 || visited[x] == color::black) continue;
        dfs(x, sigma, n+1, visited, chain);
        if (chain.size() > longestChain.size()) longestChain = chain;
        chain.clear();
    }
    delete[] sigma;
    if (longestChain.empty()) return 0;

    // Print chain
    std::cout << "Chain of length " << longestChain.size() << " found:\n";
    for (auto it = longestChain.rbegin(); it != longestChain.rend(); ++it) std::cout << *it << "->";
    std::cout << longestChain.back() << "\n";
    return *std::min_element(longestChain.begin(), longestChain.end());
}