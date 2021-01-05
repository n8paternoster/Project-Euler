// Project Euler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ctime>
#include "54.h"

int main() {
	
	clock_t t;
	t = clock();
	std::cout << numPlayer1Wins() << "\n";
	t = clock() - t;
	printf("Execution time: %f\n", ((float)t)/CLOCKS_PER_SEC);

    return 0;

}

