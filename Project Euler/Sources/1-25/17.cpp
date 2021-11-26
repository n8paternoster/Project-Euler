#include "stdafx.h"
#include "1-25/17.h"
#include <algorithm>
#include <vector>

int numberOfLettersOfNumbersUpTo(int n) {

	// In these arrays, index corresponds to a number
	int ones[10] = {0, 3, 3, 5, 4, 4, 3, 5, 5, 4};	// # of letters in "one", "two", "three", etc
	int tens[10] = {0, 4, 6, 6, 5, 5, 5, 7, 6, 6};	// # of letters in "teen", "twenty", "thirty", etc
	int hundreds[10] = {0, 13, 13, 15, 14, 14, 13, 15, 15, 14};	// # of letters in "one-hundred and", "two-hundred and", etc
	int thousands[10] = {0, 11, 11, 13, 12, 12, 11, 13, 13, 12};	// # of letters in "one thousand", "two thousand", etc

	int totalNumOfLetters = 0;

	auto numOfLetters = [&](int i) {

		// Sum the value given to every digit
		int sum = 0;
		int onesDigit = i % 10;
		int tensDigit = (i % 100) / 10;
		int hundredsDigit = (i % 1000) / 100;
		int thousandsDigit = (i % 10000) / 1000;
		sum += ones[onesDigit] + tens[tensDigit] + hundreds[hundredsDigit] + thousands[thousandsDigit];

		// Check for exceptions and adjust
		if (tensDigit == 1) {
			switch (onesDigit) {
				case 0:	sum--;	// "teen" -> "ten"
						break;
				case 1:	sum--;	// "oneteen" -> "eleven"
						break;
				case 2:	sum--;	// "twoteen" -> "twelve"
						break;
				case 3:	sum--;	// "threeteen" -> "thirteen"
						break;
				case 5: sum--;	// "fiveteen" -> "fifteen"
						break;
				case 8:	sum--;	// "eightteen" -> "eighteen"
						break;
			}
		}
		if (hundredsDigit > 0 && onesDigit == 0 && tensDigit == 0) {
			sum -= 3;			// "one-hundred and" -> "one-hundred", etc
		}

		totalNumOfLetters += sum;
	};

	std::vector<int> numbers;
	for (int i = 0; i < n; i++)
		numbers.push_back(i + 1);
	
	std::for_each(std::begin(numbers), std::end(numbers), numOfLetters);
	return totalNumOfLetters;
}
