#include "stdafx.h"
#include "1-25/25.h"
#include "bigNum.h"

uint32_t firstNDigitFibonacci(uint32_t n) {
	bigNum f("1");
	bigNum fminus1("1");
	bigNum temp;
	uint32_t index = 2;
	while (f.numDigits() < n) {
		temp = f;
		f = f + fminus1;
		fminus1 = temp;
		index++;
	}
	if (index == 2) index = 1;
	return index;
}