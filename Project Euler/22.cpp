#include "stdafx.h"
#include "22.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

/*
Using names.txt (right click and 'Save Link/Target As...'), a 46K text file containing over five-thousand first names, begin by sorting it into alphabetical order. Then working out the alphabetical value for each name, multiply this value by its alphabetical position in the list to obtain a name score.

For example, when the list is sorted into alphabetical order, COLIN, which is worth 3 + 15 + 12 + 9 + 14 = 53, is the 938th name in the list. So, COLIN would obtain a score of 938 × 53 = 49714.

What is the total of all the name scores in the file?
*/

uint64_t nameScores() {

	std::ifstream f("22.txt");
	if (!f) {
		std::cerr << "Error opening file\n";
		return 0;
	}

	std::vector<std::string> names;
	std::string name;
	while (std::getline(f,name,',')) {
		name.erase(name.begin());
		name.erase(name.end()-1);
		names.push_back(name);
	}

	std::sort(names.begin(), names.end());

	uint64_t nameScore;
	uint64_t totalScore = 0;
	for (uint64_t i = 0; i < names.size(); i++) {
		nameScore = 0;
		for (auto c : names[i])
			nameScore += c - 'A' + 1;
		nameScore *= (i + 1);
		totalScore += nameScore;
	}
	return totalScore;
}