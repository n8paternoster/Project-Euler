// Problems.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ctime>
#include "1-25/13.h"
#include "bigNum.h"
//#include "newBigNum.h"
//#include "fatNum.h"

int main() {

	clock_t t = clock();

	std::cout << p13();

	t = clock() - t;
	printf("\nExecution time: %f\n", ((float)t)/CLOCKS_PER_SEC);

    return 0;

}