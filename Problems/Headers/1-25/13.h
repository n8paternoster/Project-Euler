#pragma once

/*
	Work out the first ten digits of the sum of the following one-hundred 50-digit numbers.
	...
*/

std::string firstNDigitsOfLargeSum(int, std::string);
inline auto p13() {
	return firstNDigitsOfLargeSum(10, "Resources/13.txt");
}