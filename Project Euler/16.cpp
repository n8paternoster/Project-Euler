#include "stdafx.h"
#include "16.h"
#include "bigNum.h"

/*
215 = 32768 and the sum of its digits is 3 + 2 + 7 + 6 + 8 = 26.

What is the sum of the digits of the number 2^1000?
*/

int digitSumOfXRaisedToY(int x, int y) {
	bigNum base(std::to_string(x));
	int exp = y;
	bigNum result = base ^ exp;
	int sum = 0;
	for (int i = 0; i < result.numDigits(); i++) {
		sum += result[i];
	}
	return sum;
}