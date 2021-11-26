#include "stdafx.h"
#include "1-25/20.h"
#include "bigNum.h"

uint64_t sumOfDigitsOfNFactorial(uint64_t n) {
	bigNum num(std::to_string(n));
	bigNum fact = factorial(num);
	uint64_t result = 0;
	for (size_t i = 0; i < fact.numDigits(); i++)
		result += fact[i];
	return result;
}