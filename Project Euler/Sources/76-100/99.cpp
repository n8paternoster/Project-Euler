#include "stdafx.h"
#include "76-100/99.h"
#include <fstream>
#include <string>

int largestExponentLineNum(std::string file) {
	std::ifstream f(file);
	if (!f) {
		std::cerr << "Error opening file\n";
		return -1;
	}
	// most number of leading digits possible to calculate using int and double
	const auto n = std::min(std::numeric_limits<double>::digits10, std::numeric_limits<int>::digits10); 
	int base, exp;
	int lineNum = 1;
	int largestNumDigits = 0, largestDigits = 0, largestLineNum = -1;
	std::string buff;
	while (std::getline(f, buff, ',')) {
		base = std::stoi(buff);
		f >> exp;
		int numDigits = static_cast<int>(exp*(std::log10(base))) + 1;
		if (numDigits >= largestNumDigits) {
			// calculate first n digits of base^exp
			double x = exp * std::log10(base);
			int digits = static_cast<int>(std::floor(std::pow(10, x-std::floor(x)+n-1)));
			if (numDigits > largestNumDigits || digits > largestDigits) {
				largestNumDigits = numDigits;
				largestDigits = digits;
				largestLineNum = lineNum;
				std::cout << "Line " << lineNum << ": " << base << " ^ " << exp << " = " << digits << "... (" << numDigits << " digits)\n";
			} else if (digits == largestDigits) {
				std::cerr << "Data types are not wide enough to determine largest number\n";
				return -1;
			}
		}
		lineNum++;
	}
	return largestLineNum;
}