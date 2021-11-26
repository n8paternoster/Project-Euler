#include "stdafx.h"
#include "76-100/98.h"
#include <cctype>			// std::isalpha, std::toupper
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>	// std::unordered_multimap

using ull = unsigned long long int;
using mask = unsigned long long int;
const ull maxLength = std::numeric_limits<ull>::digits10;
struct anagramPair {
	std::string word1, word2;
	mask mask1, mask2;
};

bool isAnagram(const std::string &a, const std::string &b) {
	if (a.size() != b.size()) return false;
	int letters[26] = {};
	for (size_t i = 0; i < a.size(); i++) {
		auto &l = a[i], &r = b[i];
		if (!std::isalpha(l) || !std::isalpha(r)) return false;
		letters[std::toupper(l)-'A']++;
		letters[std::toupper(r)-'A']--;
	}
	for (auto letter : letters)
		if (letter) return false;
	return true;
}

std::pair<mask, mask> encodeAnagram(std::string str1, std::string str2) {
	/* Get a pair of masks that encode the pattern of unique letters in the anagram pair.
	*  Encoding is done by reading str1 from right to left and assigning a unique digit
	*  to each unique letter. For example: (STATE, TASTE) -> (42321, 23421) */
	if (str1.size() > maxLength || str2.size() > maxLength) {
		std::cerr << "String is too long to encode into an integer\n";
		return {};
	}
	unsigned char uniqueLetterID = '1';
	for (int i = static_cast<int>(str1.size())-1; i >= 0; i--) {
		if (uniqueLetterID > '9') {
			std::cerr << "String has more than 9 unique letters\n";
			return {};
		}
		unsigned char c = str1[i];
		if (!std::isalpha(c)) continue;
		str1[i] = uniqueLetterID;		// replace this letter with its digit ID
		for (int j = i-1; j >= 0; j--)	// replace other occurrences of c in str1
			if (str1[j] == c) str1[j] = uniqueLetterID;
		for (auto &l : str2)			// replace all occurrences of c in str2
			if (l == c) l = uniqueLetterID;
		uniqueLetterID++;
	}
	return {std::stoull(str1), std::stoull(str2)};
}

mask encodeNumber(ull num) {
	/* Encode a number in the same way as encodeAnagram but using digits instead of letters */
	ull uniqueDigitID = 1;
	mask m = 0, e = 1;
	while (num != 0) {	// check each digit from right -> left
		if (m/e % 10) {	// digit in mask already set
			num /= 10;
			e *= 10;
			continue;
		}
		ull digit = num % 10;
		m += uniqueDigitID*e;	// set this digit
		ull num2 = num/10, e2 = e*10;
		while (num2 != 0) {		// set all other occurrences of this digit
			if (num2 % 10 == digit) m += uniqueDigitID*e2;
			num2 /= 10;
			e2 *= 10;
		}
		num /= 10;
		e *= 10;
		uniqueDigitID++;
	}
	return m;
}

ull applyMask(mask mask1, ull num1, mask mask2) {
	/* Given a number (num1) and its mask (mask1), apply the same digit
	*  encoidng to mask2 to get the corresponding number (num2) */
	ull num2 = 0;
	ull e = 1;
	while (mask1 != 0) {
		ull digitID = mask1 % 10;
		ull digit = num1 % 10;
		ull e2 = 1;
		ull m2 = mask2;
		while (m2 != 0) {
			if (m2 % 10 == digitID) {
				if (num2 / e2 % 10) break;
				num2 += digit*e2;
			}
			m2 /= 10;
			e2 *= 10;
		}
		mask1 /= 10;
		num1 /= 10;
		e *= 10;
	}
	return num2;
}

ull largestAnagramSquare(std::string file) {

	// Read the words from the file
	std::ifstream f(file);
	if (!f) {
		std::cerr << "Error opening file\n";
		return 0;
	}
	std::vector<std::string> words;
	std::string buff;
	while (std::getline(f, buff, '"')) {
		if (buff == "," || buff.empty()) continue;
		words.push_back(buff);
	}

	/* Get all the anagram pairs
	*  Create a digit mask for the first word that associates a number with each letter
	*  Use the same letter->number mapping to create a second mask for the second word
	*/
	std::vector<anagramPair> anagramPairs;
	size_t longestWord = 0;
	for (size_t i = 0; i < words.size(); i++) {
		std::string &w = words[i];
		for (size_t j = i+1; j < words.size(); j++) {
			if (isAnagram(w, words[j])) {
				std::string &v = words[j];
				auto& [m1, m2] = encodeAnagram(w, v);
				anagramPairs.push_back({w, v, m1, m2});
				if (w.size() > longestWord) longestWord = w.size();
			}
		}
	}

	if (longestWord > std::numeric_limits<ull>::digits10) {
		std::cerr << "Some anagram words are too long to fit into a C++ integer type\n";
		return 0;
	}

	/* Get all square numbers up to (longestPair) number of digits
	*  Store square numbers into a map using the square number's mask as key
	*  Create a separate map for each number of digits and store them in an array
	*/
	std::vector<ull> *squares = new std::vector<ull>[longestWord+1];
	std::unordered_multimap<ull, ull> *squaresMap = new std::unordered_multimap<ull, ull>[longestWord+1];
	ull num = 1;
	ull square = num * num;
	ull squareNumDigits = static_cast<ull>(std::log10(square)) + 1;
	while (squareNumDigits <= longestWord) {
		squares[squareNumDigits].push_back(square);
		squaresMap[squareNumDigits].emplace(encodeNumber(square), square);
		num++;
		square = num*num;
		squareNumDigits = static_cast<ull>(std::log10(square)) + 1;
	}

	/* Search for square numbers that have the same mask as the first anagram word.
	*  For each match, use the digits of the square number and the second word's mask to
	*  form a second number corresponding to the second anagram word; check if this is square
	*/
	ull largestSquare = 0;
	for (const auto &p : anagramPairs) {
		auto numDigits = p.word1.size();
		auto range = squaresMap[numDigits].equal_range(p.mask1);
		for (auto &it = range.first; it != range.second; ++it) {
			ull square1 = (*it).second;
			ull square2 = applyMask(p.mask1, square1, p.mask2);
			if (std::find(squares[numDigits].begin(), squares[numDigits].end(), square2) != squares[numDigits].end()) {
				if (square1 > largestSquare) largestSquare = square1;
				if (square2 > largestSquare) largestSquare = square2;
				//std::cout << p.word1 << " (" << p.mask1 << "), " << p.word2 << " (" << p.mask2 << "): (" << square1 << ", " << square2 << ")\n";
			}
		}
	}
	delete[] squares;
	delete[] squaresMap;
	return largestSquare;
}