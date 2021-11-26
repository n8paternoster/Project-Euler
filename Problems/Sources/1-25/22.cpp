#include "pch.h"
#include "1-25/22.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

uint64_t nameScores(std::string file) {

	std::ifstream f(file);
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