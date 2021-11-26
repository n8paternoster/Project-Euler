#pragma once

/*
	The series, 1^1 + 2^2 + 3^3 + ... + 10^10 = 10405071317.

	Find the last ten digits of the series, 1^1 + 2^2 + 3^3 + ... + 1000^1000.
*/

std::string lastNDigits1000PowerSeries(unsigned n);
inline auto p48() {
	return lastNDigits1000PowerSeries(10);
}