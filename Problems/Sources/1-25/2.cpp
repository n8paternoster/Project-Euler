#include "pch.h"
#include "1-25/2.h"

int sumEvenFibonacciUpTo(int n) {
	int sum = 0;
	int temp;
	for (int i = 1, j = 2; j < n;) {
		if (j % 2 == 0) sum += j;
		temp = i; 
		i = j;
		j += temp;
	}
	return sum;
}
