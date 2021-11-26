#include "pch.h"
#include "1-25/20.h"
#include "bigNum.h"

uint64_t sumOfDigitsOfNFactorial(uint64_t n) {
	bigNum num(n);
	bigNum fact = bigNum::factorial(num);
	uint64_t result = 0;
	for (size_t i = 0; i < fact.numDigits(); i++)
		result += fact[i];
	return result;
}