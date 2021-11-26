#include "pch.h"
#include "76-100/89.h"
#include <string>
#include <cctype>	// std::toupper
#include <fstream>

// 0, 100, 200, 300, ... 0, 10, 20, 30, ... 0, 1, 2, 3, ... 8, 9
const std::string romanNumerals[] = {"", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM", "", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC", "", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"};

int romanToDecimal(std::string numeral) {
	if (numeral.empty()) return 0;
	int decimalVal = 0;
	int denomVal = 0;
	int prevDenomVal = 1'000;
	int fives[3]{};	// 'V', 'L', 'D' can only appear once each
	for (int i = 0; i < numeral.size(); i++) {
		switch (std::toupper(numeral[i])) {
			case 'I': denomVal = 1; break;
			case 'V': denomVal = 5; fives[0]++; break;
			case 'X': denomVal = 10; break;
			case 'L': denomVal = 50; fives[1]++; break;
			case 'C': denomVal = 100; break;
			case 'D': denomVal = 500; fives[2]++; break;
			case 'M': denomVal = 1000; break;
			default: throw std::invalid_argument("Invalid roman numeral");
		}
		if (denomVal > prevDenomVal) {	// check for subtractive combo
			// subtractive combo can only have 1 leading numeral
			if ((decimalVal/prevDenomVal)%10 != 1) throw std::invalid_argument("Invalid roman numeral");	
			int r = denomVal / prevDenomVal;
			if (r == 5) denomVal = (3*denomVal / 5);					// IV, XL, CD
			else if (r == 10) denomVal = (8*denomVal / 10);				// IX, XC, CM
			else throw std::invalid_argument("Invalid roman numeral");	// numerals not in descending order
		} else if (prevDenomVal/denomVal == 3 || prevDenomVal/denomVal == 8) {
			// subtractive combo cannot be followed by the same leading numeral (ex: XLX)
			throw std::invalid_argument("Invalid roman numeral");
		}
		decimalVal += denomVal;
		prevDenomVal = denomVal;
	}
	for (auto count : fives) if (count > 1) throw std::invalid_argument("Invalid roman numeral");
	return decimalVal;
}

std::string decimalToRoman(int decimal) {
	if (decimal <= 0 || decimal > 100'000) throw std::out_of_range("Decimal number is out of range");
	std::string roman = "";

	// thousands
	int count = decimal / 1'000;
	decimal -= 1'000*count;
	roman.append(count, 'M');

	// hundreds, tens, ones
	for (int i = 0, e = 100; i < 3; i++, e /= 10) {	
		int digit = (decimal / e) % 10;
		roman += romanNumerals[10*i+digit];
		decimal -= e*digit;
	}
	return roman;
}


int numExtraChars(std::string file) {

	std::ifstream f(file);
	if (!f) {
		std::cerr << "Error opening file";
		return -1;
	}

	int numReducedChars = 0;
	std::string numeral;
	while (f >> numeral) {
		std::string reduced = decimalToRoman(romanToDecimal(numeral));
		numReducedChars += (numeral.size() - reduced.size());
	}

	return numReducedChars;
}
