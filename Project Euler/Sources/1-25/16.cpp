#include "stdafx.h"
#include "1-25/16.h"
#include "bigNum.h"

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