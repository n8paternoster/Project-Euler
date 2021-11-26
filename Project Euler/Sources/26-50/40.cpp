#include "stdafx.h"
#include "26-50/40.h"

unsigned nthDigitOfChampernowneConstant(unsigned n) {

	// Let z be the integer in Champernowne's constant that index n points to

	// gives the index in Champernowne's constant of the first number that is numDigits long (ex: for numDigits = 3, returns index 190 which contains the "1" of the number 100)
	auto I10N = [](unsigned numDigits) -> unsigned {
		return (9*numDigits*(unsigned)pow(10, numDigits-1) - (unsigned)pow(10, numDigits) + 10) / 9;
	};

	// Calculate the number of digits in z
	unsigned zLength = 1;
	while (I10N(zLength+1) <= n) zLength++;

	// Find z by taking the first number of length zLength and adding the offset from n
	unsigned z = (unsigned)pow(10,zLength-1) + (n-I10N(zLength))/zLength;

	// Calculate which digit in z is pointed to by n
	unsigned j = (n - I10N(zLength)) % zLength;	// index in z
	unsigned digit = (unsigned)(z/(unsigned)pow(10, zLength-1-j)) % 10;

	return digit;
}
