#include "stdafx.h"
#include "51-75/62.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>	// std::sort

// original brute force solution checks each generated cube with each other generated so far with the same number of digits and returns when the first group of size N is found
// performance is slow, time complexity on the order of O(n^2)
/*
bool isPermutation(unsigned long long p, unsigned long long q) {
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

unsigned long long smallestCubeWithNCubicPermutations(unsigned N) {
	std::vector<std::vector<unsigned long long>> groups;
	std::vector<unsigned long long> result;
	bool resultFound = false;
	unsigned numDigits = 4;	// 10^3 = 1000
	for (unsigned long long x = 10; x <= 9000; x++) {
		unsigned long long cube = (unsigned long long)pow(x, 3);
		if ((unsigned)log10(cube)+1 > numDigits) {
			numDigits++;
			groups.clear();
		}
		bool isGroupMember = false;
		for (auto &group : groups) {
			if (isPermutation(cube, group[0])) {
				group.push_back(cube);
				isGroupMember = true;
				if (group.size() == N) {
					result = group;
					resultFound = true;
				}
				break;
			}
		}
		if (resultFound) break;
		if (!isGroupMember) groups.emplace_back(std::vector<unsigned long long>{cube});
	}
	if (resultFound) {
		for (auto cube : result)
			std::cout << cube << " ";
		std::cout << "\n";
		return result.front();
	}
	return 0;
}
*/

// optimized solution converts each generated cube to a string of digits, sorts the digits, and uses this as a key to a hash table; each entry in the hash table stores a vector of numbers that make up the permutation group, returns when the first group reaches a size of N
// time complexity on the order of O(n)
unsigned long long smallestCubeWithNCubicPermutations(unsigned N, unsigned long long maxBase) {
	if (maxBase > 2'600'000ull) maxBase = 2'600'000ull;		// avoid overflow
	std::unordered_map<std::string, std::vector<unsigned long long>> groups;
	unsigned numDigits = 1;	// 1^3 = 1
	std::vector<unsigned long long> smallest;
	for (unsigned long long x = 1; x <= maxBase; x++) {
		unsigned long long cube = x*x*x;
		if ((unsigned long long)log10(cube)+1 > numDigits) {
			if (!smallest.empty()) {	// check all cubes with the same amount of digits to get the group with the lowest member
				std::cout << "Smallest group found: ";
				for (auto n : smallest) std::cout << "(" << n << ") ";
				std::cout << "\n";
				return smallest.front();
			}
			groups.clear();
			numDigits++;
		}
		std::string key = std::to_string(cube);
		std::sort(key.begin(), key.end());
		groups[key].push_back(cube);
		if (groups[key].size() == N) {
			if (smallest.empty()) smallest = groups[key];
			else if (groups[key].front() < smallest.front()) smallest = groups[key];
		}
	}
	if (!smallest.empty()) {	// permutation group of size N was found but not all cubes with this number of digits could be checked
		std::cout << "Group found: ";
		for (auto n : smallest) std::cout << "(" << n << ") ";
		std::cout << "\n";
		return smallest.front();
	}
	return 0;
}
