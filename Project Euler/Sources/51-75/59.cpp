#include "stdafx.h"
#include "51-75/59.h"
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>		// isalpha, tolower

using CHAR = unsigned char;
using Letter_Counts = std::unordered_map<CHAR, double>;

// Returns the amount of times each letter is expected to appear in a text of specified length
Letter_Counts getExpected(unsigned dataLength) {
	CHAR letters[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
	double freqs[26] = {.0855, .016, .0316, .0387, .121, .0218, .0209, .0496, .0733, .0022, .0081, .0421, .0253, .0717, .0747, .0207, .001, .0633, .0673, .0894, .0268, .0106, .0183, .0019, .0172, .0011};
	Letter_Counts vals;
	for (unsigned i = 0; i < 26; i++)
		vals.emplace(letters[i], (dataLength * freqs[i]));
	return vals;
}

/* Quantifies how closely the frequency of letters in "text" matches the expected average distribution of english letters, this test measures the goodness-of-fit between the theoretical frequency of letters and the observed frequency */
double chiSquared(std::vector<CHAR> text) {

	// count the observed frequency of letters
	Letter_Counts observed;
	unsigned numLetters = 0;
	for (auto c : text) {
		if (isalpha(c)) {
			observed[tolower(c)]++;
			numLetters++;
		}
	}
	// calculate the expected frequency of letters
	Letter_Counts expected = getExpected(numLetters);

	// calculate the chi-squared value of the text
	double chiSquared = 0.0;
	for (CHAR c = 'a'; c <= 'z'; c++) {
		double degree = (observed[c] - expected[c]) * (observed[c] - expected[c]) / expected[c];
		chiSquared += degree;
	}
	return chiSquared;
}

unsigned sumOfCipherDecryption(std::string file) {
	std::vector<CHAR> cipherText;
	std::ifstream f(file);
	if (!f) {
		std::cerr << "Error opening cipher file\n";
		return 0;
	}
	std::string s;
	while (std::getline(f, s, ',')) cipherText.push_back(static_cast<CHAR>(std::stoul(s)));
	f.close();

	// key is 3 chars long, split the cipher into three columns
	std::vector<std::vector<CHAR>> cols(3);
	int i = 0;
	for (auto c : cipherText) {
		cols[i].push_back(c);
		if (++i > 2) i = 0;
	}

	// determine the key one char at a time, each char of the key corresponds to one column of the cipher
	CHAR key[3] = {};
	int k = 0;
	for (auto col : cols) {
		std::vector<CHAR> plainCol;

		// try each char for this col and select the one that produces plaintext that most closely matches the letter frequencies of the English language
		double lowestChi = 1'000'000.0;
		CHAR keyC;
		for (char testC = 'a'; testC <= 'z'; testC++) {
			plainCol.clear();
			for (auto c : col) plainCol.push_back(static_cast<CHAR>(c ^ testC));
			double chi = chiSquared(plainCol);
			if (chi < lowestChi) {
				lowestChi = chi;
				keyC = testC;
			}
		}

		key[k] = keyC;
		k++;
	}

	// use the key on the cipher to get the plaintext
	std::vector<CHAR> plainText;
	k = 0;
	for (auto c : cipherText) {
		plainText.push_back(static_cast<CHAR>(c ^ key[k]));
		if (++k > 2) k = 0;
	}

	// add the ascii values of the chars in the plaintext
	unsigned sum = 0;
	for (auto c : plainText) sum += c;

	// print
	std::cout << "Key found: " << key[0] << key[1] << key[2] << "\n";
	std::cout << "Ciphertext (ASCII values):\n\n";
	for (auto c : cipherText) std::cout << (int)c << ",";
	std::cout << "\n\nPlaintext:\n\n";
	for (auto c : plainText) std::cout << c;
	std::cout << "\n\n";

	return sum;
}
