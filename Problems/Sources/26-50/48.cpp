#include "pch.h"
#include "26-50/48.h"
#include <string>

// All numbers stored as an array of shorts. Long multiplication and addition operations are performed only up to the first n digits
std::string lastNDigits1000PowerSeries(unsigned n) {
	
	if (n < 1) return std::string("");

	short *sum = new short[n]();
	short *partialPower = new short[n]();
	short *product = new short[n]();

	for (short t = 1; t <= 1000; t++) {

		// Express the current term t as an array of digits
		short tLength = (short)log10(t)+1;
		short *term = new short[tLength]();
		for (short i = 0; i < n; i++) {
			short digit = (short)(t/pow(10, i)) % 10;
			partialPower[i] = digit;
			if (i < tLength) term[i] = digit;
		}

		// Calculate t^t by performing t successive multiplications
		for (short i = 1; i < t; i++) {

			// Multiply term by partialPower
			for (short a = 0; a < n && a < tLength; a++) {
				short b, carry = 0;
				for (b = 0; b < n && b < n; b++) {
					if (a+b >= n) break;
					product[a+b] += term[a] * partialPower[b] + carry;
					carry = product[a+b] / 10;
					product[a+b] %= 10;
				}
				if (a+b < n) product[a+b] = carry;
			}
			// Store the result in partialPower
			for (short j = 0; j < n; j++) {
				partialPower[j] = product[j];
				product[j] = 0;
			}
		}

		// Add t^t to the result
		short i, carry = 0;
		for (i = 0; i < n; i++) {
			short digit = partialPower[i] + sum[i] + carry;
			if (digit > 9) {
				digit -= 10;
				carry = 1;
			} else carry = 0;
			sum[i] = digit;
		}
		if (i < n) sum[i] = carry;

		delete[] term;
	}

	std::string result;
	for (short i = 0; i < n; i++)
		result.push_back(sum[n-1-i] + '0');

	delete[] sum;
	delete[] partialPower;
	delete[] product;

	return result;
}
