#include "stdafx.h"
#include "13.h"
#include <fstream>
#include <string>
#include "bigNum.h"

/*
	Work out the first ten digits of the sum of the following one-hundred 50-digit numbers.
	...
*/


std::string firstNDigitsOfLargeSum(int n) {
	std::ifstream f("13.txt");
	if (!f) {
		std::cerr << "Error opening file\n;";
		return "";
	}
	bigNum numbers[100];
	std::string number;
	int i = 0;
	while (std::getline(f, number)) {
		numbers[i] = bigNum(number);
		i++;
	}
	f.close();

	bigNum result;
	for (int i = 0; i < 100; i++)
		result = result + numbers[i];

	std::string resultString;
	for (int i = result.numDigits() - 1; i > result.numDigits() - 1 - n; i--)
		resultString.append(std::to_string(result[i]));
	return resultString;
}