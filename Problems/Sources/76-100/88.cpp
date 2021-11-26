#include "pch.h"
#include "76-100/88.h"
#include <vector>
#include <set>

/* 
    Instead of iterating through each value of k, check consecutive integers (num) to see which values of k they satisfy. First generate all the divisors of num up to sqrt(num). Then generate all valid sets of numbers that multiply to num using these divisors and cache the sums of these sets and their number of elements. Finally add 1's to these sets so they also sum to num and mark their set size (k) as found
*/
int minimalProductSumNumberSum(int maxK) {

    // Find the minimum product-sum number for every set of size 2 up to size k
    std::set<int> minProductSums;
    
    // Track which values of k have been solved
    std::vector<bool> kFound(maxK+1, false);
    int totalK = maxK-1;    // all k from [2, maxK] inclusive
    int kCount = 0;

    // Cache the divisors of each number and the sums and num of elements of each set that multiply to that num
    std::vector<std::vector<int>> divisors;
    std::vector<std::set<std::pair<int, int>>> divisorSums; // {setSum, setCount}
    int num = 0;
    while (kCount < totalK) {

        // Get the divisors of num
        int root = (int)std::sqrt(num);
        divisors.push_back(std::vector<int>());
        for (int i = 2; i <= root; i++) {
            if (num % i == 0) divisors[num].push_back(i);
        }

        // Get all sets of integers whose product is num
        divisorSums.push_back(std::set<std::pair<int, int>>());
        for (auto smallDiv : divisors[num]) {
            int largeDiv = num/smallDiv;
            for (auto s : divisorSums[largeDiv])
                divisorSums[num].insert({smallDiv+s.first, s.second+1});
            divisorSums[num].insert({smallDiv+largeDiv, 2});
        }

        // Add 1's to every set to get all sets whose product and sum is num; mark all set sizes (k values) for this number as found
        int setSize = 0;
        for (auto sum : divisorSums[num]) {
            if (sum.first > num) break;
            setSize = sum.second + (num - sum.first);
            if (setSize > maxK) continue;
            if (!kFound[setSize]) {
                kCount++;
                kFound[setSize] = true;
                minProductSums.insert(num);
            }
        }
        num++;
    }

    int result = 0;
    for (auto minNum : minProductSums)
        result += minNum;
    std::cout << "Max num required: " << num << "\n";

    return result;
}
