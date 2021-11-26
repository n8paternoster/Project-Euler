#include "pch.h"
#include "1-25/1.h"

int sumOfMultiplesOf3And5UpTo(int n) {
	int sum = 0;
	for (int i = 0; i < n; i++)
		if (i % 3 == 0 || i % 5 == 0) sum += i;
	return sum;
}