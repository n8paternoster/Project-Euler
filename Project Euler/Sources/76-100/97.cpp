#include "stdafx.h"
#include "76-100/97.h"

/* Modular multiply and modular power functions from https://en.wikipedia.org/wiki/Modular_arithmetic#Example_implementations
* These functions make use of the fact that left shifting a binary number by n is equivalent to multiplying by 2^n and unsigned types "wrap around" to 0 when they overflow; specifically 64-bit unsigned integers are modulo divided by 2^64 when a calculation overflows.
*/

// multiply a*b (mod m) with no overflow in the intermediate operations
uint64_t multiplyModM(uint64_t a, uint64_t b, uint64_t m) {
	static const uint64_t max = static_cast<uint64_t>(std::sqrt(std::numeric_limits<uint64_t>::max()));
	if (a < max && b < max)
		return a * b % m;
	uint64_t p = 0;
	a %= m;
	b %= m;
	// iterate over the binary representation of a from msb -> lsb
	for (int i = 0; i < 64; i++) {		 
		p <<= 1;						// p *= 2
		if (p >= m) p -= m;				// p %= m
		if (a & 0x8000000000000000ull)	// 64 bit mask for the msb
			p += b;
		if (p >= m) p -= m;				// p %= m
		a <<= 1;						// a *= 2
	}
	return p;
}

// b^e (mod m)
uint64_t powModM(uint64_t b, uint64_t e, uint64_t m) {
	uint64_t result = (m == 1) ? 0 : 1;
	while (e > 0) {
		if (e & 1) result = multiplyModM(result, b, m);
		e >>= 1;
		b = multiplyModM(b, b, m);
	}
	return result;
}

uint64_t lastNDigitsMersennePrime(uint64_t n) {
	if (n > std::numeric_limits<uint64_t>::digits10) {
		std::cerr << "Too many digits specified\n";
		return 0;
	}
	uint64_t mod = static_cast<uint64_t>(std::pow(10, n));

	// 2004 Mersenne prime: 28,433*2^(7,830,456)+1
	const uint64_t a = 28'433, base = 2, exp = 7'830'457, c = 1;
	
	uint64_t result = powModM(base, exp, mod);	// result = 2^(7,830,457)
	result = multiplyModM(result, a, mod);		// result *= 28,433
	result += c;								// result += 1
	result %= mod;
	return result;
}