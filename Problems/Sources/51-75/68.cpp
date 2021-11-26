#include "pch.h"
#include "51-75/68.h"
#include <string>
#include <vector>
#include <algorithm>

struct Triple {
	int x;
	int y;
	int z;
	void clear() {
		x = 0;
		y = 0;
		z = 0;
	}
};

bool operator==(const Triple& lhs, const Triple& rhs) {
	return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
}

std::ostream& operator<<(std::ostream& os, const Triple& t) {
	os << "(" << t.x << ", " << t.y << ", " << t.z << ")";
	return os;
}

bool containsVal(const Triple& t, int val) {
	return (t.x == val || t.y == val || t.z == val);
}

std::pair<int, int> otherVals(const Triple& t, int val) {
	if (t.x == val) return {t.y, t.z};
	else if (t.y == val) return {t.x, t.z};
	else if (t.z == val) return {t.x, t.y};
	else return {0, 0};
}

std::string solutionToString(std::vector<Triple>& triples, std::vector<int>& count) {

	std::vector<int> leaves;
	for (int b = 1; b < count.size(); b++)
		if (count[b] == 1) leaves.push_back(b);

	// Start with the triplet that has the smallest leaf, choose the larger of the two shared nodes to follow
	int leaf = *std::min_element(leaves.begin(), leaves.end());
	auto it = std::search_n(triples.begin(), triples.end(), 1, leaf, containsVal);
	std::pair<int, int> others = otherVals(*it, leaf);
	int firstShared = others.first > others.second ? others.first : others.second;
	int secondShared = others.first > others.second ? others.second : others.first;
	std::string str = std::to_string(leaf) += std::to_string(firstShared) += std::to_string(secondShared);
	it->clear();
	
	// Work clockwise, find the next triplet
	it = std::search_n(triples.begin(), triples.end(), 1, secondShared, containsVal);
	while (it != triples.end()) {
		firstShared = secondShared;
		others = otherVals(*it, firstShared);
		leaf = count[others.first] == 1 ? others.first : others.second;
		secondShared = count[others.first] == 1 ? others.second : others.first;
		str += std::to_string(leaf) += std::to_string(firstShared) += std::to_string(secondShared);
		it->clear();
		it = std::search_n(triples.begin(), triples.end(), 1, secondShared, containsVal);
	}
	return str;
}

std::string maxMagicNgonString(size_t n, size_t maxStringLength) {
	if (n < 3) return "";
	int numIntegers = 2*n;	// the n-gon contains integers [1, 2*n]
	int minSum = 2*n + 3;
	int maxSum = 4*n;
	std::string result = "";

	for (int sum = minSum; sum <= maxSum; sum++) {

		// Generate all possible triples that add to this sum
		std::vector<Triple> triples;
		int i = 1, j = sum-numIntegers-1, k = numIntegers;	// initial triple
		while (j < k) {
			while (j < k) {
				triples.push_back({i, j, k});
				j++;
				k--;
			}
			i++;
			j = i+1;
			k = sum - i - j;
			if (k > numIntegers) {
				j += k-numIntegers;
				k -= k-numIntegers;
			}
		}
		
		/* Search for a group of n triples that satisfy the following conditions:
				(1) n integers appear exactly once each (the 'leaf' nodes)
				(2) the other n integers appear exactly twice each (the 'shared' nodes)
				(3) each triple contains exactly one 'leaf' integer */
		if (triples.size() < n) continue;
		std::vector<bool> mask(triples.size());	// bitmask used to check each combination of triples
		std::vector<int> count(numIntegers+1);	// counts the # of appearances of each integer, not 0-indexed
		for (size_t b = 0; b < n; b++)			// set initial mask to the first n triples
			mask[b] = true;
		do {
			// Count the number of appearances of each integer from 1 to 2*n
			count.assign(numIntegers+1, 0);
			for (int b = 0; b < mask.size(); b++) {
				if (mask[b]) {
					Triple t = triples[b];
					count[t.x]++;
					count[t.y]++;
					count[t.z]++;
				}
			}
			// Check for (1), (2)
			int numLeaves = 0, numShared = 0;
			for (auto it = count.begin()+1; it != count.end(); ++it) {
				if (*it == 1) numLeaves++;
				else if (*it == 2) numShared++;
				else break;
			}
			if (numLeaves == n && numShared == n) {

				// Check for (3)
				bool isSolution = true;
				for (int b = 0; b < mask.size(); b++) {
					if (mask[b]) {
						Triple t = triples[b];
						int appearanceCount = 0;
						appearanceCount += count[t.x] + count[t.y] + count[t.z];
						if (appearanceCount != 5) {
							isSolution = false;
							break;
						}
					}
				}
				if (isSolution) {
					std::vector<Triple> solutionTriples;
					for (int b = 0; b < mask.size(); b++)
						if (mask[b]) solutionTriples.push_back(triples[b]);
					std::string solution = solutionToString(solutionTriples, count);
					if (solution.length() <= maxStringLength || maxStringLength == 0) {
						if (solution.length() > result.length()) result = solution;
						else if (solution.length() == result.length() && result < solution) result = solution;
					}
					//std::cout << "Solution found for sum = " << sum << "\n";
					//std::cout << solution << "\n";
				}
			}
		} while (std::prev_permutation(mask.begin(), mask.end()));
	}

	return result;
}
