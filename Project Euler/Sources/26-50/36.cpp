#include "stdafx.h"
#include "26-50/36.h"
#include <string>

bool isPalindrome(unsigned n) {
	bool returnVal = true;
	unsigned length = (unsigned)log10(n)+1;
	for (int i = 0; i < length/2; i++) {
		if ((n/(unsigned)(pow(10, i))%10 != (n/(unsigned)(pow(10, length-1-i)))%10)) {
			returnVal = false;
			break;
		}
	}
	return returnVal;
}

bool isPalindrome(std::string n) {
	bool returnVal = true;
	for (int i = 0; i < n.length()/2; i++) {
		if (n[i] != n[n.length()-1-i]) {
			returnVal = false;
			break;
		}
	}
	return returnVal;
}

std::string toBinary(unsigned n) {
	std::string result;
	while (n != 0) {
		result = (n % 2 == 0 ? "0" : "1") + result;
		n >>= 1;
	}
	return result;
}

unsigned sumDoubleBasePalindromesUnderN(unsigned N) {
	unsigned sum = 0;
	for (unsigned n = 1; n < N; n += 2) {	// even numbers end in 0 in binary, cannot be palindromes
		if (isPalindrome(n) && isPalindrome(toBinary(n))) {
			std::cout << n << " = " << toBinary(n) << "\n";
			sum += n;
		}
	}
	return sum;
}
