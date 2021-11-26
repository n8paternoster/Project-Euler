#include "stdafx.h"
#include "1-25/4.h"
#include <string>

bool intIsPalindrome(int n) {
	std::string s = std::to_string(n);
	for (int i = 0, j = s.length() - 1; i < j; i++, j--)
		if (s[i] != s[j]) return false;
	return true;
}

int largestPalindromeProductOfTwoXDigitNumbers(int numDigits) {
	std::string max = std::string(numDigits, '9');
	std::string min = "1" + std::string(numDigits - 1, '0');
	int maxNumber = std::stoi(max);
	int minNumber = std::stoi(min);
	if (maxNumber > INT32_MAX / maxNumber) {
		std::cout << "Number of digits too large.\n";
		return -1;
	}
	int maxProduct = maxNumber * maxNumber;
	int minProduct = minNumber * minNumber;
	for (int i = maxProduct; i >= minProduct; i--) {		// find the largest palindromic number
		if (intIsPalindrome(i)) {
			for (int j = minNumber; j < maxNumber; j++) {	// check if it is a product of two 3-digit numbers
				if (i % j == 0 && i / j >= minNumber && i / j <= maxNumber) {
					std::cout << i / j << " x " << j << " = " << i << "\n";
					return i;
				}
			}
		}
	}
	return -1;
}