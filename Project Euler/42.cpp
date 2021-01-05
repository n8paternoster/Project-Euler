#include "stdafx.h"
#include "42.h"
#include <string>
#include <fstream>
#include <vector>

unsigned numTriangleWords() {

	std::ifstream f("42.txt");
	if (!f) {
		std::cerr << "Error opening file\n;";
		return 0;
	}
	
	std::vector<std::string> words;
	std::string word;
	while (std::getline(f, word, ',')) words.push_back(word);

	auto isTriangle = [](int value) -> bool {
		int n = 1, t = 1;
		while ((t = (n * (n+1) / 2)) < value) n++;
		return t == value;
	};

	unsigned count = 0;
	for (auto s : words) {
		int value = 0;
		for (auto c : s) value += (c == '"') ? 0 : (c - 'A' + 1);
		if (isTriangle(value)) count++;
	}

	f.close();

	return count;
}
