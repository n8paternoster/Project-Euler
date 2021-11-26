#include "bigNum.h"
#include <iostream>
#include <limits>			// std::digits10
#include <unordered_map>	// hash table to store remainders during division
#include <algorithm>		// std::max

const uint64_t karatsubaMinDigits = 256;
static int count = 0;

/* ------------------------------------------------------ */
/*                      num_type                          */
/* -------------------------------------------------------*/

// Special member functions
bigNum::num_type::num_type():
	digits(new digit_type[mDefaultSize]()), size(mDefaultSize), lsb(0), msb(0), isPositive(true)
{ }
bigNum::num_type::num_type(size_type sz) :
	lsb(0), msb(0), isPositive(true)
{
	if (sz < 1) sz = mDefaultSize;
	size = sz;
	digits = new digit_type[size]();
}
bigNum::num_type::num_type(const num_type &other):
	digits(new digit_type[other.size]()), size(other.size), lsb(other.lsb), msb(other.msb), isPositive(other.isPositive) {
	for (index_type i = 0; i < other.size; i++)
		digits[i] = other.digits[i];
}
bigNum::num_type::num_type(num_type &&other) noexcept:
	digits(other.digits), size(other.size), lsb(other.lsb), msb(other.msb), isPositive(other.isPositive)
{
	other.digits = nullptr;
}
bigNum::num_type& bigNum::num_type::operator=(const num_type &other) {
	if (this == &other) return *this;
	if (size != other.size) {
		delete[] digits;
		size = other.size;
		digits = new digit_type[other.size]();
	}
	for (index_type i = 0; i < other.size; i++)
		digits[i] = other.digits[i];
	lsb = other.lsb;
	msb = other.msb;
	isPositive = other.isPositive;
	return *this;
}
bigNum::num_type& bigNum::num_type::operator=(num_type &&other) noexcept {
	if (this == &other) return *this;
	delete[] digits;
	digits = other.digits;
	size = other.size;
	lsb = other.lsb;
	msb = other.msb;
	isPositive = other.isPositive;
	other.digits = nullptr;
	return *this;
}
bigNum::num_type::~num_type() {
	delete[] digits;
}

// Utility
bigNum::digit_type bigNum::num_type::operator[](index_type i) const {
	if (i > size || i < 0 || i > (msb - lsb)) throw std::range_error("Index out of range");
	return digits[msb-i];
}
bigNum::digit_type& bigNum::num_type::operator[](index_type i) {
	if (i > size || i < 0 || i > (msb - lsb)) throw std::range_error("Index out of range");
	return digits[msb-i];
}
void bigNum::num_type::clear() {
	for (index_type i = 0; i < size; i++)
		digits[i] = 0;
	lsb = 0;
	msb = 0;
	isPositive = true;
}
void bigNum::num_type::resize(index_type sz) {
	if (sz < 0 || sz < size) return;	// can only resize to a larger size
	if (sz > mMaxDigits) throw std::overflow_error("Overflow in resize");
	size_type newSize = static_cast<size_type>(sz);
	digit_type *newDigits = new digit_type[newSize]();
	for (index_type i = 0; i < size; i++)
		newDigits[i] = digits[i];
	delete[] digits;
	size = newSize;
	digits = newDigits;
}
bool bigNum::num_type::toInteger(index_type &out) const {
	index_type numDigits = msb - lsb + 1;
	constexpr index_type max = std::numeric_limits<index_type>::max();
	index_type maxNumDigits = static_cast<index_type>(std::log10(max)+1);
	if (numDigits < maxNumDigits) {
		out = 0;
		for (index_type i = lsb, e = 1; i <= msb; i++, e *= 10)
			out += e * digits[i];
		if (!isPositive) out = -out;
		return true;
	} else if (numDigits == maxNumDigits) {
		// only some values of this length are allowed, check digit by digit
		index_type num = 0;
		index_type e = static_cast<index_type>(std::pow(10, maxNumDigits-1));
		bool check = true;
		for (index_type i = msb; i > lsb; i--) {
			if (check) {	// check digits if all preceding digits are equal to max
				if (digits[i] > max/e % 10) return false;
				else if (digits[i] < max/e % 10) check = false;
			}
			num += e * digits[i];
			e /= 10;
		}
		// check the least significant digit
		index_type l = digits[lsb];
		if (check) {		// the maximum lsb changes depending on sign
			index_type maxLSB = max % 10;
			if (!isPositive) {
				constexpr index_type min = std::numeric_limits<index_type>::min();
				maxLSB = min / 10;
				maxLSB = min - 10*maxLSB;
			}
			if (l > std::abs(maxLSB)) return false;
		}
		if (!isPositive) {
			num = -num;
			l = -l;
		}
		num += l;
		out = num;
		return true;
	}
	return false;
}
bool bigNum::num_type::isZero() const {
	index_type z = msb;
	while (digits[z] == 0 && z > lsb) z--;
	return (z == lsb && digits[z] == 0);
}

// Arithmetic
bigNum::index_type bigNum::num_type::addSubtract(const num_type &rhs, index_type rhsShift, bool add, size_type precision) {

	// Calculate the number of digits needed for the result
	index_type lhsNumDigits = msb - lsb + 1;
	index_type rhsNumDigits = rhs.msb - rhs.lsb + 1 + rhsShift;	// # of integer digits
	if (rhsNumDigits < 1) rhsNumDigits = 1;
	index_type resultNumDigits = std::max(lhsNumDigits, rhsNumDigits);
	if (rhsShift < 0) resultNumDigits += -rhsShift;	// if rhs is shifted right, result has fraction digits
	if (resultNumDigits+1 > mMaxDigits) throw std::overflow_error("Overflow in addSubtract");

	// Allocate new memory for the result if
	//	(1) there is not enough space, or
	//	(2) calculation of the result in-place will overwrite later digits (when rhs is shifted right)
	digit_type *result = digits;
	size_type resultSize = std::max(static_cast<size_type>(resultNumDigits)+1, size);
	if (precision > resultSize) resultSize = precision;
	if (resultSize > size || rhsShift < 0)
		result = new digit_type[resultSize]();

	// Adjust the start index of the number with the larger lsb to account for rhsShift
	const num_type &lhs = *this;
	index_type lhsStart = lhs.lsb - (rhsShift < 0 ? -rhsShift : 0);
	index_type rhsStart = rhs.lsb - (rhsShift > 0 ? rhsShift : 0);

	index_type resultMSB = resultNumDigits-1;
	if (add == (lhs.isPositive == rhs.isPositive)) {
		// Add the digits
		digit_type carry = 0;
		for (index_type i = 0, l = lhsStart, r = rhsStart; i < resultNumDigits; i++, l++, r++) {
			digit_type left = (l < lhs.lsb || l > lhs.msb) ? 0 : lhs.digits[l];
			digit_type right = (r < rhs.lsb || r > rhs.msb) ? 0 : rhs.digits[r];
			digit_type d = left + right + carry;
			if (d > 9) {
				d -= 10;
				carry = 1;
			} else carry = 0;
			result[i] = d;
		}
		if (carry) {	// addition resulted in an additional digit
			resultMSB++;
			result[resultMSB] = carry;
		}
	} else {
		// Subtract the digits
		digit_type carry = 0;
		for (index_type i = 0, l = lhsStart, r = rhsStart; i < resultNumDigits; i++, l++, r++) {
			digit_type left = (l < lhs.lsb || l > lhs.msb) ? 0 : lhs.digits[l];
			digit_type right = (r < rhs.lsb || r > rhs.msb) ? 0 : rhs.digits[r];
			digit_type d = (9 - left) + right + carry;
			if (d > 9) {
				d -= 10;
				carry = 1;
			} else carry = 0;
			result[i] = d;
		}
		if (carry) {	// result of subtraction is negative, |lhs| < |rhs|
			isPositive = !isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			index_type i = 0;
			while (carry && i < resultNumDigits) {
				result[i] += carry;
				if (result[i] > 9) result[i] -= 10;
				else carry = 0;
				i++;
			}
		} else {		// result of subtraction is positive, |lhs| >= |rhs|
			for (index_type i = 0; i < resultNumDigits; i++)
				result[i] = 9 - result[i];
		}
	}
	while (result[resultMSB] == 0 && resultMSB > 0) resultMSB--;

	// Set the result
	index_type lhsShift = 0;
	if (resultMSB == 0 && result[resultMSB] == 0) {	// check if result is 0
		isPositive = true;
	} else if (precision) {
		lhsShift = resultMSB - precision + 1;
		if (lhsShift > 0) {			// copy starting from the lsb
			for (index_type i = 0; i <= resultMSB; i++)
				result[i] = (i < precision) ? result[i+lhsShift] : 0;
		} else if (lhsShift < 0) {	// copy starting from the msb
			for (index_type i = resultMSB; i >= lhsShift; i--)
				result[i-lhsShift] = (i >= 0) ? result[i] : 0;
		}
		resultMSB = precision-1;
	}
	if (result == digits) {	// no new data was was allocated, clear any unchanged digits
		index_type m = msb;
		while (m > resultMSB) digits[m--] = 0;
	} else {				// new data was allocated
		delete[] digits;
		digits = result;
		size = resultSize;
	}
	lsb = 0;
	msb = resultMSB;
	return lhsShift;
}
bigNum::index_type bigNum::num_type::longMultiply(const num_type &rhs, size_type precision) {
	// INPUT: number to multiply (rhs), precision of result not rounded (precision)
	// OUTPUT: product (*this), decimal point shift of the product (shift)

	// Allocate space for the product
	index_type prodNumDigits = (msb - lsb + 1) + (rhs.msb - rhs.lsb + 1);	// lhsNumDigits + rhsNumDigits
	if (prodNumDigits > mMaxDigits) throw std::overflow_error("Overflow in longMultiply");
	size_type prodSize = std::max(precision ? precision : static_cast<size_type>(prodNumDigits), size);
	digit_type *prod = new digit_type[prodSize]();

	// Calculate the product
	index_type shift = precision ? prodNumDigits-precision-1 : 0;
	index_type sum = 0;
	for (index_type p = 0; p < prodNumDigits-1; p++) {	// for each digit of the product
		for (index_type r = (lsb+p <= msb) ? 0 : (lsb+p)-msb; r <= p && rhs.lsb+r <= rhs.msb; r++)
			sum += digits[lsb+(p-r)] * rhs.digits[rhs.lsb+r];
		if (p >= shift) prod[p-shift] = sum % 10;
		sum /= 10;
	}
	if (sum % 10 != 0) {
		if (precision) {
			for (index_type i = (shift < 0 ? -shift-1 : 0); i < precision-1; i++)
				prod[i] = prod[i+1];
			prod[precision-1] = sum % 10;
			shift++;
		} else prod[prodNumDigits-1] = sum % 10;
	}

	// Set the result
	delete[] digits;
	digits = prod;
	size = prodSize;
	lsb = 0;
	msb = precision ? precision-1 : prodNumDigits-1;
	while (digits[msb] == 0 && msb > lsb) msb--;
	isPositive = (isPositive == rhs.isPositive);

	return shift;
}
bigNum::index_type bigNum::num_type::divideDigits(div_mode mode, num_type &rhs, digit_type *quotient, index_type quotNumDigits, index_type quotNumIntegerDigits, size_type precision) {
	// INPUT: dividend (*this), divisor (rhs), 
	// OUTPUT: dividend/divisor (quotient), 
	// Divide this by rhs and store the result in quotient
	// Return the index of the quotient's least significant digit that was set
	num_type &lhs = *this;

	// Simpler algorithm when the divisor is one digit
	index_type rhsNumDigits = rhs.msb - rhs.lsb + 1;
	if (rhsNumDigits == 1) {
		digit_type d = rhs.digits[lsb];
		if (d == 0) throw std::invalid_argument("Division by zero");
		index_type q = quotNumDigits-1;	// quotient index
		index_type p = 0;				// count number of precise digits
		digit_type r = 0, qDigit = 0;
		for (index_type i = 0, j = msb; i < quotNumDigits && q >= 0; i++, j--, q--) {
			digit_type div = 10*r + (j >= lsb ? digits[j] : 0);	// 2 digit partial dividend
			qDigit = div / d;
			r = div % d;
			quotient[q] = qDigit;

			// Check for termination
			if (mode == div_mode::DEFAULT) {
				if (i >= quotNumIntegerDigits) {	// start counting precision after integer digits
					// Check for a zero remainder
					if (r == 0) {
						if (qDigit == 0) q++;
						break;
					}
					if (p > 0 || qDigit || quotNumIntegerDigits > 0) p++;
					if (p >= 9) break;
				}
			} else if (mode == div_mode::PRECISION) {
				if (p > 0 || qDigit) p++;
				if (p >= precision) break;
			}
		}
		return q;
	}

	// Helper function for single digit multiplication
	auto multiplyByDigit = [](num_type &num, digit_type d) {
		if (d == 1) return;
		digit_type carry = 0;
		for (index_type i = num.lsb; i <= num.msb; i++) {
			num.digits[i] = num.digits[i] * d + carry;
			carry = num.digits[i] / 10;
			num.digits[i] %= 10;
		}
		if (carry) {
			if (num.msb+1 >= num.size) num.resize(static_cast<index_type>(num.size)+1);
			num.digits[num.msb+1] = carry;
			num.msb++;
		}
	};

	// Normalize lhs and rhs so that the first digit of rhs is >= 5 and lhs has an additional digit
	digit_type d = 10 / (rhs.digits[rhs.msb] + 1);	// normalization factor
	index_type lMSB = lhs.msb;
	multiplyByDigit(lhs, d);
	multiplyByDigit(rhs, d);
	if (lhs.msb == lMSB) {	// add a zero to the lhs
		if (lhs.msb+1 >= lhs.size) lhs.resize(static_cast<index_type>(lhs.size)+1);
		lhs.digits[lhs.msb+1] = 0;
		lhs.msb++;
	}

	// Allocate and initialize the partial dividend
	index_type n = rhsNumDigits + 1;	// size of partial dividend
	if (n > mMaxDigits) throw std::overflow_error("Overflow in longDivide");
	digit_type *div = new digit_type[static_cast<size_type>(n)]();
	index_type l = lhs.msb;
	for (index_type i = n-1; i >= 0 && l >= lhs.lsb; i--, l--)
		div[i] = lhs.digits[l];

	// Cache the products of the multiplication of the divisor by each digit
	num_type *products[10]{};

	// Divide u/v according to div_mode:
	// 		DEFAULT: divide to get an integer quotient then divide to the default decimal precision, stopping if the quotient terminates or repeats
	//		PRECISION: divide to the specified precision
	//		EUCLID: divide to get an integer quotient
	digit_type *u = lhs.digits;
	digit_type *v = rhs.digits;
	index_type p = 0;				// count number of precise digits
	index_type q = quotNumDigits-1;	// quotient index

	for (index_type i = 0; i < quotNumDigits && q >= 0; i++, q--) {
		// Divide the partial dividend (div) by the divisor (v) to get one digit (qHat) of the quotient

		// Calculate qHat
		digit_type qHat = (10*div[n-1]+div[n-2]) / v[rhs.msb];
		digit_type rHat = (10*div[n-1]+div[n-2]) % v[rhs.msb];
		while (qHat >= 10 || qHat*v[rhs.msb-1] > (10*rHat+div[n-3])) {
			qHat--;
			rHat += v[rhs.msb];
			if (rHat >= 10) break;
		}

		// Multiply v by qHat and subtract from the partial dividend
		if (!products[qHat]) {
			products[qHat] = new num_type(rhs);
			multiplyByDigit(*products[qHat], qHat);
		}
		num_type &product = *products[qHat];
		digit_type carry = 0;
		for (index_type j = 0, k = product.lsb; j < n; j++, k++) {
			div[j] = (9 - div[j]) + product.digits[k] + carry;
			if (div[j] > 9) {
				carry = 1;
				div[j] -= 10;
			} else carry = 0;
		}
		if (carry) {	// result is negative - q was 1 too large
			qHat--;
			// "End-around carry" - add the carry to the LSB to get the difference (a negative value)
			for (index_type j = 0; j < n; j++) {
				div[j] += carry;
				if (div[j] > 9) div[j] -= 10;
				else break;
			}
			// Add back v to get the correct partial quotient
			carry = 0;
			for (index_type j = 0, k = rhs.lsb; j < n-1; j++, k++) {
				div[j] += (9 - v[k]) + carry;
				if (div[j] > 9) {
					carry = 1;
					div[j] -= 10;
				} else carry = 0;
			}
			div[n-1] = 9;	// carry will always be 0 for the last digit
		}
		for (index_type j = 0; j < n; j++)
			div[j] = 9 - div[j];

		// Set this digit of the quotient
		quotient[q] = qHat;

		// Check for termination
		if (mode == div_mode::DEFAULT) {
			if (i >= quotNumIntegerDigits) {	// start counting precision after integer digits
				// Check for a zero remainder
				bool zeroRem = true;
				index_type j = 0;
				while (zeroRem && j < n) if (div[j++] != 0) zeroRem = false;
				if (zeroRem) { if (qHat == 0) q++; break; }
				if (p > 0 || qHat || quotNumIntegerDigits > 0) p++;
				if (p >= mDefaultDivPrecision) break;
			}
		} else if (mode == div_mode::PRECISION) {
			if (p > 0 || qHat) p++;
			if (p >= precision) break;
		}

		// Shift the partial dividend
		for (index_type j = n-1; j > 0; j--)
			div[j] = div[j-1];
		div[0] = (l >= lhs.lsb) ? u[l--] : 0;
	}

	// Free memory for the partial dividend and cached products
	delete[] div;
	for (index_type i = 0; i < 10; i++) delete products[i];

	return q;
}
bigNum::index_type bigNum::num_type::longDivide(num_type rhs, index_type rhsShift, div_mode mode, size_type precision) {

	// Check for division by zero and bad precision
	if (rhs.isZero()) throw std::invalid_argument("Division by zero");
	if (mode == div_mode::PRECISION && precision == 0) throw std::invalid_argument("Invalid precision in longDivide");

	// Return val, the shift required to make the quotient an integer
	index_type lhsShift = 0;
	num_type &lhs = *this;

	// Calculate the number of integer digits in the quotient (using rhsShift)
	index_type lhsNumDigits = lhs.msb - lhs.lsb + 1;
	index_type rhsNumDigits = rhs.msb - rhs.lsb + 1;
	index_type quotNumIntegerDigits = lhsNumDigits - (rhsNumDigits + rhsShift) + 1;
	if (quotNumIntegerDigits < 0) quotNumIntegerDigits = 0;	// lhs < rhs

	// Get the total number of quotient digits to calculate, depending on div_mode
	index_type quotNumDigits = quotNumIntegerDigits;
	if (mode == div_mode::DEFAULT) quotNumDigits += (mDefaultDivPrecision + 1);
	else if (mode == div_mode::PRECISION) quotNumDigits = static_cast<index_type>(precision) + 1;
	if (quotNumDigits > mMaxDigits) throw std::overflow_error("Overflow in longDivide");

	// Allocate memory for the quotient
	size_type quotSize = (quotNumDigits > size) ? static_cast<size_type>(quotNumDigits) : size;
	digit_type *quotient = new digit_type[quotSize]();

	// Divide rhs into lhs and store the result in quotient
	index_type qLSB = lhs.divideDigits(mode, rhs, quotient, quotNumDigits, quotNumIntegerDigits, precision);

	// Set the result
	if (qLSB < 0) qLSB = 0;
	index_type numDigitsSet = quotNumDigits - qLSB;
	index_type numIntDigits = lhsNumDigits - rhsNumDigits + 1;	// ignoring rhsShift
	lhsShift = numIntDigits - numDigitsSet;
	switch (mode) {
		case div_mode::EUCLID:
			if (quotNumIntegerDigits <= 0) lhsShift = 0;
			[[fallthrough]];
		case div_mode::DEFAULT:
		case div_mode::PRECISION:
			delete[] digits;
			digits = quotient;
			size = quotSize;
			lsb = qLSB;
			msb = quotNumDigits ? (quotNumDigits-1) : 0;
			isPositive = (isPositive == rhs.isPositive);

			// Shift lsb to index 0
			while (digits[msb] == 0 && msb > lsb) msb--;
			if (lsb > 0) {
				index_type i = 0;
				for (index_type j = lsb; i < size && j <= msb && j < size; i++, j++)
					digits[i] = digits[j];
				while (i <= msb) digits[i++] = 0;
				msb -= lsb;
				lsb = 0;
			}
			break;
		default:
			delete[] quotient;
			lhsShift = 0;
	}

	return lhsShift;
}
bigNum::index_type bigNum::num_type::integerPow(index_type exp, size_type precision) {

	num_type &lhs = *this;

	// Check for zero/one operands
	bool lZero = lhs.isZero();
	if (lZero && exp == 0) {		// 0 ^ 0
		throw std::invalid_argument("Indeterminate form in integerPow");
	} else if (lZero && exp < 0) {	// 1 / (0^exp)
		throw std::invalid_argument("Division by zero in integerPow");
	} else if (lZero) {				// 0 ^ exp = 0
		lhs.clear();
		return 0;
	} else if (exp == 0) {			// lhs ^ 0 = 1
		lhs.clear();
		lhs.digits[0] = 1;
		return 0;
	} else if (lhs.msb == lhs.lsb && lhs.digits[lhs.msb] == 1) {	// +-1 ^ exp = +-1
		if (!lhs.isPositive && std::abs(exp) % 2 == 0)				// -1 ^ exp = +1
			lhs.isPositive = true;
		return 0;							
	} else if (exp == 1) {			// lhs ^ 1 = lhs
		return 0;
	} else if (exp == -1) {			// lhs ^ -1 = 1 / lhs
		div_mode mode = precision ? div_mode::PRECISION : div_mode::DEFAULT;
		num_type l = lhs;
		lhs.clear();
		lhs.digits[0] = 1;
		return lhs.longDivide(l, 0, mode, precision);
	}

	// Get the binary representation of the exponent
	index_type e = std::abs(exp);
	constexpr auto size = std::numeric_limits<index_type>::digits + 1;
	int binary[size] = {};
	int b = 0;
	while (e != 0) {
		binary[b++] = e % 2;
		e /= 2;
	}

	// Calculate exponentiation by squaring
	index_type numDigits = lhs.msb - lhs.lsb + 1;
	num_type base = lhs;
	for (b -= 2; b >= 0; b--) {	// skip the leading 1
		if (numDigits > karatsubaMinDigits) lhs.karatsubaMultiply(lhs, 0);
		else lhs.longMultiply(lhs, 0);
		if (binary[b]) {
			if (numDigits > karatsubaMinDigits) lhs.karatsubaMultiply(base, 0);
			else lhs.longMultiply(base, 0);
		}
		numDigits = lhs.msb - lhs.lsb + 1;
	}

	// If the exponent is negative, take the inverse
	index_type lhsShift = 0;
	if (exp < 0) {
		div_mode mode = precision ? div_mode::PRECISION : div_mode::DEFAULT;
		num_type l = lhs;
		lhs.clear();
		lhs.digits[0] = 1;
		lhsShift = lhs.longDivide(l, 0, mode, precision);
	}

	// Otherwise if precision was specified adjust the number of digits in the result
	else if (precision) {
		if (precision > size) resize(precision);
		index_type shift = msb - precision + 1;
		if (shift > 0) {		// copy starting from the lsb
			for (index_type i = 0; i <= msb; i++)
				digits[i] = (i < precision) ? digits[i+shift] : 0;
		} else if (shift < 0) {	// copy starting from the msb
			for (index_type i = msb; i >= shift; i--)
				digits[i-shift] = (i >= 0) ? digits[i] : 0;
		}
		msb = precision-1;
		lhsShift += shift;
	}

	return lhsShift;
}

// Karatsuba multiplication
std::pair<bigNum::num_type, bigNum::num_type> bigNum::num_type::splitAt(index_type m) const {
	if (m < 0 || m > mMaxDigits) throw std::overflow_error("Invalid index in splitAt");
	if (m == 0) return {num_type(size), num_type(*this)};
	if (m >= msb-lsb+1) return {num_type(*this), num_type(size)};
	num_type low(size), high(size);
	index_type l = 0, h = 0;
	for (index_type i = lsb; i <= msb; i++) {
		if (i < lsb+m) low.digits[l++] = digits[i];
		else high.digits[h++] = digits[i];		// digit at index m belongs to high
	}
	low.msb = l ? l-1 : 0;
	high.msb = h ? h-1 : 0;
	return {low, high};
}
bigNum::num_type bigNum::num_type::karatsubaIter(const num_type &a, const num_type &b) {

	// Base case: For operands under 256 digits, use long multiplication
	index_type aNumDigits = a.msb - a.lsb + 1;
	index_type bNumDigits = b.msb - b.lsb + 1;
	if (aNumDigits < karatsubaMinDigits && bNumDigits < karatsubaMinDigits) {
		num_type result(a);
		result.longMultiply(b, 0);
		return result;
	}

	// Choose m, the position to split each number
	index_type n = (aNumDigits > bNumDigits) ? aNumDigits : bNumDigits;
	index_type m = n / 2;

	// Split a and b into smaller numbers with approximately half the number of digits
	auto [aLow, aHigh] = a.splitAt(m);
	auto [bLow, bHigh] = b.splitAt(m);

	// Compute three intermediate values z0, z1, z2 from numbers of approximately half size
	num_type z0 = karatsubaIter(aLow, bLow);	// z0 = aLow * bLow
	num_type z2 = karatsubaIter(aHigh, bHigh);	// z1 = aHigh * bHigh
	aLow.addSubtract(aHigh, 0, true, 0);
	bLow.addSubtract(bHigh, 0, true, 0);
	num_type z1 = karatsubaIter(aLow, bLow);	// z2 = (aLow + aHigh) * (bLow + bHigh)

	// a*b = z2*(10^2m) + (z1-z2-z0)*(10^m) + z0
	z1.addSubtract(z0, 0, false, 0);	// z1 -= z2
	z1.addSubtract(z2, 0, false, 0);	// z1 -= z0
	z0.addSubtract(z1, m, true, 0);		// z0 += (z1 shifted by 10^m)
	z0.addSubtract(z2, 2*m, true, 0);	// z0 += (z2 shifted by 10^2m)
	return z0;
}
bigNum::index_type bigNum::num_type::karatsubaMultiply(const num_type &rhs, size_type precision) {

	// Multiply
	bool prodSign = (isPositive == rhs.isPositive);
	*this = karatsubaIter(*this, rhs);
	isPositive = prodSign;

	// Check for zero
	if (msb == lsb && digits[msb] == 0) {
		isPositive = true;
		return 0;
	}

	// Check if precision was specified
	index_type prodShift = 0;
	if (precision) {
		if (precision > size) resize(precision);
		prodShift = msb - precision + 1;
		if (prodShift > 0) {		// copy starting from the lsb
			for (index_type i = 0; i <= msb; i++)
				digits[i] = (i < precision) ? digits[i+prodShift] : 0;
		} else if (prodShift < 0) {	// copy starting from the msb
			for (index_type i = msb; i >= prodShift; i--)
				digits[i-prodShift] = (i >= 0) ? digits[i] : 0;
		}
		msb = precision-1;
	}

	return prodShift;
}

/* ------------------------------------------------------ */
/*						 bigNum                           */
/* -------------------------------------------------------*/

// Constructors
bigNum::bigNum():
	mSignificand(), mExponent(), mRepLSB(-1), mRepMSB(-1) {}
bigNum::bigNum(size_type sigSize, size_type expSize) :
	mSignificand(sigSize), mExponent(expSize), mRepLSB(-1), mRepMSB(-1) {}
bigNum::bigNum(std::string num) :
	mSignificand(), mExponent(), mRepLSB(-1), mRepMSB(-1) {
	if (num.length() == 0) return;	// zero-init with an empty string

	// Check if the number is negative
	if (num[0] == '-') {
		mSignificand.isPositive = false;
		num.erase(0, 1);
	}

	// Check if the number contains a decimal point
	size_t tenthsPlaceIndex = num.length();		// index of first fraction digit
	for (size_t i = 0; i < num.length(); i++) {
		if (num[i] == '.' || num[i] == ',') {
			tenthsPlaceIndex = i;
			num.erase(i, 1);
			break;
		}
	}

	// Get the number of digits in the string, ignoring leading and trailing zeros
	if (num.length() == 0) throw std::invalid_argument("BigNum cannot be constructed with no digits");
	size_t msb = 0, lsb = num.length()-1;
	while (num[msb] == '0' && msb < num.length()-1) msb++;
	while (num[lsb] == '0' && lsb > msb) lsb--;
	size_t numDigits = lsb - msb + 1;
	if (numDigits > mMaxDigits) throw std::length_error("Overflow error in bigNum(std::string)");

	// Set the significand
	size_type numSigDigits = static_cast<size_type>(numDigits);
	if (numSigDigits > mSignificand.size) mSignificand.resize(numSigDigits);
	index_type i = 0;
	for (size_t j = 0; j < numDigits && i < mSignificand.size; j++, i++) {
		char c = num[lsb-j];
		if (c < '0' || c > '9') throw std::invalid_argument("Number contains an invalid character");
		mSignificand.digits[i] = c - '0';	// digits are stored little-endian
	}
	mSignificand.lsb = 0;
	mSignificand.msb = i-1;

	// Check if the number is zero
	if (mSignificand.lsb == mSignificand.msb && mSignificand.digits[mSignificand.lsb] == 0) {
		mSignificand.isPositive = true;
		return;
	}

	// Get the value of the exponent
	size_t exp = 0;
	if ((lsb + 1) <= tenthsPlaceIndex) {
		exp = tenthsPlaceIndex - (lsb + 1);
		mExponent.isPositive = true;
	} else {
		exp = (lsb + 1) - tenthsPlaceIndex;
		mExponent.isPositive = false;
	}

	// Set the exponent
	size_type numExpDigits = (exp == 0) ? 1 : static_cast<size_type>(std::log10(exp)) + 1;
	if (numExpDigits > mExponent.size) mExponent.resize(numExpDigits);
	i = 0;
	do {
		mExponent.digits[i++] = exp % 10;
		exp /= 10;
	} while (exp != 0 && i < mExponent.size);
	mExponent.lsb = 0;
	mExponent.msb = i-1;
}

// Utility
void bigNum::printFloat() const {
	if (mSignificand.msb >= mSignificand.size || mSignificand.lsb < 0 || mExponent.msb >= mExponent.size || mExponent.lsb < 0) {
		std::cerr << "No digits found\n";
		return;
	}

	// Print the significand
	if (!mSignificand.isPositive) std::cout << "-";
	for (index_type i = mSignificand.msb; i >= mSignificand.lsb; i--) {
		if (i == mRepMSB) std::cout << "(";
		std::cout << mSignificand.digits[i];
		if (i == mRepLSB) std::cout << ")";
	}

	// Print the exponent
	std::cout << "e";
	if (!mExponent.isPositive) std::cout << "-";
	for (index_type i = mExponent.msb; i >= mExponent.lsb; i--)
		std::cout << mExponent.digits[i];
}
void bigNum::print(index_type firstNDigits) const {
	if (mSignificand.msb >= mSignificand.size || mSignificand.lsb < 0 || mExponent.msb >= mExponent.size || mExponent.lsb < 0) {
		std::cerr << "No digits found\n";
		return;
	}

	// If the number is too large, print in floating-point notation
	const index_type maxExpPrintSize = 1'000;
	index_type exp = 0;
	if (!mExponent.toInteger(exp) || exp > maxExpPrintSize) {
		printFloat();
		return;
	}

	// Calculate the total number of digits to print
	index_type numSigDigits = mSignificand.msb - mSignificand.lsb + 1;
	index_type numLeftPaddedDigits = (exp < 0 && -exp >= numSigDigits) ? (-exp-numSigDigits+1) : 0;
	index_type numRightPaddedDigits = (exp > 0) ? exp : 0;
	index_type numDigits = numSigDigits + numLeftPaddedDigits + numRightPaddedDigits;

	// Calculate the indexes of the decimal point and repetend position
	index_type onesPlaceIndex = (exp > 0) ? /*numSigDigits*/numDigits-1 : (numSigDigits + exp - 1);
	if (onesPlaceIndex < 0) onesPlaceIndex = 0;
	index_type repSize = 0, repStart = -1, repEnd = -1;
	if (mRepLSB >= 0) {
		repSize = mRepMSB - mRepLSB + 1;
		repStart = mSignificand.msb - mRepMSB + numLeftPaddedDigits;
		if (repStart <= onesPlaceIndex)			// repetend must be right of the decimal point
			repStart += (onesPlaceIndex-repStart+1);
		repEnd = repStart + repSize - 1;

		// shift the repetend indexes to the left as much as possible
		index_type i = mRepMSB+1, j = mRepLSB;
		while (repStart-1 > onesPlaceIndex) {
			if (i <= mSignificand.msb) {	// shift the repetend left if the digit in the significand left of the rep matches the rep's least significant digit
				if (mSignificand.digits[i] == mSignificand.digits[j]) {
					repStart--; repEnd--;
					i++; j++;
				} else break;
			} else if (mSignificand.digits[j] == 0) {	// shift the repetend left if the rep ends in a 0 (since a 0 to the left will be added with scaling)
				repStart--; repEnd--;
				i++; j++;
			} else break;
		}

		// check if additional digits need to be printed (the repetend must be entirely right of the radix)
		if (repEnd >= numDigits) {
			numRightPaddedDigits += (repEnd-numDigits+1);
			numDigits += (repEnd-numDigits+1);
		}
	}

	// Check if a specific # of digits was requested
	if (firstNDigits > 0 && firstNDigits < numDigits) {
		std::cout << "Displaying the first " << firstNDigits << " digit" << (firstNDigits > 1 ? "s\n" : "\n");
		numDigits = firstNDigits;
	} else firstNDigits = 0;

	// Print the number
	if (!mSignificand.isPositive) std::cout << "-";
	for (index_type i = 0, s = mSignificand.msb, r = mRepMSB; i < numDigits; i++) {
		if (i == repStart) std::cout << "(";

		// Print the i'th digit of the number
		if (exp >= 0) {
			if (i < numSigDigits && s >= mSignificand.lsb)	// print digits of the significand
				std::cout << mSignificand.digits[s--];
			else if (repSize > 0) {							// print extra digits from the repetend
				std::cout << mSignificand.digits[r--];
				if (r < mRepLSB) r = mRepMSB;
			} else std::cout << "0";							// print extra 0's from scaling
		} else {
			if (i < numLeftPaddedDigits)					// print extra digits from scaling
				std::cout << "0";
			else if (s >= mRepLSB)							// print digits of the significand
				std::cout << mSignificand.digits[s--];
			else if (repSize > 0) {							// print extra digits from the repetend
				std::cout << mSignificand.digits[r--];
				if (r < mRepLSB) r = mRepMSB;
			}
		}

		if (i == repEnd) { std::cout << ")"; break; }
		if (i == onesPlaceIndex && i+1 < numDigits) std::cout << ".";
	}

	// Indicate that not all of the digits have been printed
	if (firstNDigits > 0) std::cout << "...";
}
bool bigNum::integerDifference(const bigNum::num_type &lhs, const bigNum::num_type &rhs, index_type &out) {

	// Check if both numbers can be converted to an integer type
	index_type lInt = 0, rInt = 0;
	if (lhs.toInteger(lInt) && rhs.toInteger(rInt)) {
		// Check for overflow/underflow first
		constexpr index_type MAX = std::numeric_limits<index_type>::max();
		constexpr index_type MIN = std::numeric_limits<index_type>::min();
		if ((rInt < 0) && (lInt > MAX + rInt)) return false;	// overflow
		if ((rInt > 0) && (lInt < MIN + rInt)) return false;	// underflow
		out = lInt - rInt;
		return true;
	}

	// If not, calculate the difference digit by digit
	num_type diff(lhs);
	diff.addSubtract(rhs, 0, false, 0);
	index_type diffInt = 0;
	if (!diff.toInteger(diffInt)) return false;
	out = diffInt;
	return true;
}
bool bigNum::toInteger(index_type &out) const {

	// Convert the exponent to an integer type
	index_type exp = 0;
	if (!mExponent.toInteger(exp)) return false;						// exponent is too large
	if (exp > std::numeric_limits<int64_t>::digits10) return false;		// exponent is too large

	// Convert the significand to an integer type and multiply by 10^exp
	index_type sig = 0;
	if (exp < 0) {
		index_type shift = -exp;
		if (mSignificand.lsb+shift > mSignificand.msb) return false;	// not an integer; all of the significand digits are fractional
		for (index_type i = mSignificand.lsb; i < mSignificand.lsb+shift; i++)
			if (mSignificand.digits[i]) return false;					// not an integer; at least 1 significand fractional digit is not 0
		num_type sigShifted = mSignificand;
		sigShifted.lsb += shift;
		if (!sigShifted.toInteger(sig)) return false;					// number is too large
	} else {
		if (!mSignificand.toInteger(sig)) return false;					// significand is too large
		if (exp > 0) {
			index_type m = static_cast<index_type>(std::pow(10, exp));
			if (std::abs(sig) > std::numeric_limits<index_type>::max() / m) return false;	// number is too large
			sig *= m;
		}
	}
	out = sig;
	return true;
}
void bigNum::addExponent(index_type val) {
	// add val to mExponent in-place

	if (val == 0) return;
	bool numIsPositive = (val > 0);
	if (val < 0) val = -val;
	index_type valNumDigits = static_cast<index_type>(std::log10(val)) + 1;
	if (mExponent.lsb+valNumDigits+1 > mExponent.size)
		mExponent.resize(mExponent.lsb+valNumDigits+1);

	if (mExponent.isPositive == numIsPositive) {			// addition
		index_type i = mExponent.lsb;
		digit_type carry = 0;
		while ((carry || val) && i < mExponent.size) {
			if (i > mExponent.msb) mExponent.digits[i] = 0;
			mExponent.digits[i] += (val % 10) + carry;
			if (mExponent.digits[i] > 9) {
				mExponent.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
			val /= 10;
			i++;
		}
		if (i-1 > mExponent.msb) mExponent.msb = i-1;
	} else {											// subtraction
		index_type expNumDigits = mExponent.msb - mExponent.lsb + 1;
		index_type diffNumDigits = (expNumDigits > valNumDigits) ? expNumDigits : valNumDigits;
		digit_type carry = 0;
		for (index_type i = mExponent.lsb, j = 0; j < diffNumDigits; i++, j++) {
			mExponent.digits[i] = (9 - (i > mExponent.msb ? 0 : mExponent.digits[i])) + (val % 10) + carry;
			if (mExponent.digits[i] > 9) {
				mExponent.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
			val /= 10;
		}
		if (carry) {	// result of subtraction is negative
			mExponent.isPositive = !mExponent.isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			index_type i = mExponent.lsb;
			while (carry && i < mExponent.size) {
				mExponent.digits[i] += carry;
				if (mExponent.digits[i] > 9) mExponent.digits[i] -= 10;
				else carry = 0;
				i++;
			}
		} else {		// result of subtraction is positive
			for (index_type i = mExponent.lsb, j = 0; j < diffNumDigits; i++, j++)
				mExponent.digits[i] = 9 - mExponent.digits[i];
		}
		mExponent.msb = mExponent.lsb + diffNumDigits - 1;
		while (mExponent.digits[mExponent.msb] == 0 && mExponent.msb > mExponent.lsb) mExponent.msb--;
	}
	if (mExponent.msb == mExponent.lsb && mExponent.digits[mExponent.lsb] == 0) mExponent.isPositive = true;	 // check for -zero
}
void bigNum::multiplyExponent(index_type val) {
	// multiply mExponent by val

	if (val == 1) return;
	else if (val == 0) {
		mExponent.clear();
		return;
	}
	bool valIsPositive = (val >= 0);
	if (val < 0) val = -val;
	index_type valNumDigits = static_cast<index_type>(std::log10(val)) + 1;
	index_type expNumDigits = mExponent.msb - mExponent.lsb + 1;
	index_type prodNumDigits = valNumDigits + expNumDigits;
	if (prodNumDigits > mMaxDigits) throw std::overflow_error("Overflow in multiplyExponent");
	size_type prodSize = std::max(static_cast<size_type>(prodNumDigits), mExponent.size);
	digit_type *prod = new digit_type[prodSize]();

	// Long multiplication
	for (index_type i = 0; i < valNumDigits; i++) {							// for each digit in val
		digit_type v = val % 10, carry = 0;
		for (index_type j = 0, l = mExponent.lsb; j < expNumDigits; j++) {	// for each digit in mExponent
			prod[i+j] += carry + v * mExponent.digits[l+j];
			carry = prod[i+j] / 10;
			prod[i+j] %= 10;
		}
		prod[i+expNumDigits] = carry;
		val /= 10;
	}

	delete[] mExponent.digits;
	mExponent.digits = prod;
	mExponent.size = prodSize;
	mExponent.lsb = 0;
	mExponent.msb = prodNumDigits-1;
	while (mExponent.digits[mExponent.msb] == 0 && mExponent.msb > mExponent.lsb) mExponent.msb--;
	mExponent.isPositive = (mExponent.isPositive == valIsPositive);
	if (mExponent.msb == mExponent.lsb && mExponent.digits[mExponent.lsb] == 0) mExponent.isPositive = true;	 // check for -zero
}
void bigNum::round(index_type sigDigits, bool precise) {
	index_type offset = mSignificand.msb-mSignificand.lsb+1 - sigDigits;
	if (sigDigits <= 0 || offset <= 0) return;

	// Copy the significant digits, rounding while the least digit is >= 5
	digit_type carry = (mSignificand.digits[mSignificand.lsb+offset-1] >= 5);
	index_type l = 0, m = sigDigits-1;
	for (index_type i = 0; i < sigDigits; i++) {
		mSignificand.digits[i] = mSignificand.digits[mSignificand.lsb+offset+i] + carry;
		if (mSignificand.digits[i] > 9) {
			mSignificand.digits[i] -= 10;
			carry = 1;
		} else carry = 0;
	}
	if (carry) {
		m++;
		l++;
		mSignificand.digits[sigDigits] = carry;
		offset++;
	}

	// Remove trailing zeros
	if (!precise)							
		while (mSignificand.digits[l] == 0 && l < m) {
			l++;
			offset++;
		}

	// Clear extra digits
	for (index_type i = mSignificand.msb; i > m; i--)
		mSignificand.digits[i] = 0;

	// Align the number to begin at index 0
	if (l != 0) {							
		for (index_type i = 0; l+i <= m; i++)
			mSignificand.digits[i] = mSignificand.digits[i+l];
		m -= l;
		l = 0;
	}

	// Check for zero
	if (l == m && mSignificand.digits[l] == 0) mSignificand.isPositive = true;

	mSignificand.lsb = 0;
	mSignificand.msb = m;
	if (offset) addExponent(offset);
}
std::ostream& operator<<(std::ostream &out, const bigNum &num) {
	// If a precision is set, print num in its floating point representation, otherwise print normally
	if (num.mPrecision) num.printFloat();
	else num.print();
	return out;
}
bigNum::operator int() const {
	// Truncates fraction digits, returns 0 on failure
	index_type sig = 0, exp = 0;
	if (!mSignificand.toInteger(sig) || !mExponent.toInteger(exp)) return 0;
	constexpr index_type max = std::numeric_limits<int>::max();
	constexpr index_type min = std::numeric_limits<int>::min();
	while (exp != 0 && sig != 0) {
		if (exp > 0) {
			if (sig > max || sig < min) return 0;
			sig *= 10;
			exp--;
		} else {
			sig /= 10;
			exp++;
		}
	}
	if (sig > max || sig < min) return 0;
	return static_cast<int>(sig);
}

// Comparison operators
bool bigNum::operator==(const bigNum &rhs) const {
	const bigNum &lhs = *this;

	index_type l = lhs.mSignificand.msb;
	index_type r = rhs.mSignificand.msb;

	// Check if either significand is zero and compare their signs
	if (l >= lhs.mSignificand.size || lhs.mSignificand.lsb < 0 || r >= rhs.mSignificand.size || rhs.mSignificand.lsb < 0) {
		std::cerr << "Error reading operands in operator==";
		return false;
	}
	bool lIsZero = true;
	for (; l >= lhs.mSignificand.lsb; l--)
		if (lhs.mSignificand.digits[l] != 0) { lIsZero = false; break; }
	if (l < lhs.mSignificand.lsb) l = lhs.mSignificand.lsb;
	bool rIsZero = true;
	for (; r >= rhs.mSignificand.lsb; r--)
		if (rhs.mSignificand.digits[r] != 0) { rIsZero = false; break; }
	if (r < rhs.mSignificand.lsb) r = rhs.mSignificand.lsb;
	if (lIsZero && rIsZero) return true;
	else if (lIsZero != rIsZero) return false;
	else if (lhs.mSignificand.isPositive != rhs.mSignificand.isPositive) return false;

	// Get the significand sizes ignoring any leading zeros
	index_type lSigSize = l - lhs.mSignificand.lsb + 1;
	index_type rSigSize = r - rhs.mSignificand.lsb + 1;

	// Check if the numbers have different orders of magnitude
	// They have the same order of magnitude if the difference of their exponents is the opposite of the difference of their significand lengths
	index_type sigSizeDiff = rSigSize - lSigSize;
	index_type expDiff = 0;
	if (!integerDifference(lhs.mExponent, rhs.mExponent, expDiff)) return false;
	if (sigSizeDiff != expDiff) return false;	// numbers have different orders of magnitude

	// Check all significand digits
	for (; l >= lhs.mSignificand.lsb && r >= rhs.mSignificand.lsb; l--, r--)
		if (lhs.mSignificand.digits[l] != rhs.mSignificand.digits[r]) return false;
	index_type lRep = lhs.mRepMSB;
	index_type rRep = rhs.mRepMSB;
	while (l >= lhs.mSignificand.lsb) {	// check the remaining lhs digits
		digit_type rDigit = 0;
		if (rRep >= 0) {
			rDigit = rhs.mSignificand.digits[rRep];
			if (--rRep < rhs.mRepLSB) rRep = rhs.mRepMSB;
		}
		if (lhs.mSignificand.digits[l] != rDigit) return false;
		l--;
	}
	while (r >= rhs.mSignificand.lsb) {	// check the remaining rhs digits
		digit_type lDigit = 0;
		if (lRep >= 0) {
			lDigit = lhs.mSignificand.digits[lRep];
			if (--lRep < lhs.mRepLSB) lRep = lhs.mRepMSB;
		}
		if (rhs.mSignificand.digits[r] != lDigit) return false;
		r--;
	}

	// Check if either number has a repetend
	if (lhs.mRepLSB >= 0 || rhs.mRepLSB >= 0) {
		if (lhs.mRepLSB >= 0 && rhs.mRepLSB < 0) {			// only lhs has a repetend
			for (lRep = lhs.mRepMSB; lRep >= lhs.mRepLSB; lRep--)
				if (lhs.mSignificand.digits[lRep] != 0) return false;
		} else if (rhs.mRepLSB >= 0 && lhs.mRepLSB < 0) {	// only rhs has a repetend
			for (rRep = rhs.mRepMSB; rRep >= rhs.mRepLSB; rRep--)
				if (rhs.mSignificand.digits[rRep] != 0) return false;
		} else {											// both have a repetend
			index_type lRepSize = lhs.mRepMSB - lhs.mRepLSB + 1;
			index_type rRepSize = rhs.mRepMSB - rhs.mRepLSB + 1;

			// line up the repetends if the significands are different lengths
			index_type lRepStart = lhs.mRepMSB, rRepStart = rhs.mRepMSB;
			index_type lSigSize = lhs.mSignificand.msb - lhs.mSignificand.lsb + 1;
			index_type rSigSize = rhs.mSignificand.msb - rhs.mSignificand.lsb + 1;
			if (lSigSize > rSigSize) rRepStart -= (lSigSize-rSigSize) % rRepSize;
			else if (rSigSize > lSigSize) lRepStart -= (rSigSize-lSigSize) % lRepSize;

			// extend each number's repetend, the most extra digits you need to compare is the lcm(lRepSize, rRepSize)
			index_type numExtraDigits = std::lcm(lRepSize, rRepSize);
			lRep = lRepStart;
			rRep = rRepStart;
			for (index_type i = 0; i < numExtraDigits; i++) {
				if (lhs.mSignificand.digits[lRep] != rhs.mSignificand.digits[rRep]) return false;
				if (--lRep < lhs.mRepLSB) lRep = lhs.mRepMSB;
				if (--rRep < rhs.mRepLSB) rRep = rhs.mRepMSB;
			}
		}
	}
	return true;
}
bool bigNum::operator<(const bigNum &rhs) const {
	const bigNum &lhs = *this;

	index_type l = lhs.mSignificand.msb;
	index_type r = rhs.mSignificand.msb;

	// Find the first non-zero digit of each significand and check if either is zero
	if (l >= lhs.mSignificand.size || lhs.mSignificand.lsb < 0 || r >= rhs.mSignificand.size || rhs.mSignificand.lsb < 0) {
		std::cerr << "Error reading operands in operator==";
		return false;
	}
	bool lIsZero = true;
	for (; l >= lhs.mSignificand.lsb; l--)
		if (lhs.mSignificand.digits[l] != 0) { lIsZero = false; break; }
	if (l < lhs.mSignificand.lsb) l = lhs.mSignificand.lsb;
	bool rIsZero = true;
	for (; r >= rhs.mSignificand.lsb; r--)
		if (rhs.mSignificand.digits[r] != 0) { rIsZero = false; break; }
	if (r < rhs.mSignificand.lsb) r = rhs.mSignificand.lsb;

	// Compare signs
	if (lIsZero && rIsZero) return false;
	else if (lIsZero) return rhs.mSignificand.isPositive;	// (lhs is 0) true if rhs is positive
	else if (rIsZero) return !lhs.mSignificand.isPositive;	// (rhs is 0) true if lhs is negative
	else if (lhs.mSignificand.isPositive != rhs.mSignificand.isPositive)
		return rhs.mSignificand.isPositive;		// (signs don't match) true if rhs is positive
	bool sign = lhs.mSignificand.isPositive;	// sign of both numbers

	// Get the significand sizes ignoring any leading zeros
	index_type lSigSize = l - lhs.mSignificand.lsb + 1;
	index_type rSigSize = r - rhs.mSignificand.lsb + 1;

	// Check if the numbers have different orders of magnitude
	// They have the same order of magnitude if the difference of their exponents is the opposite of the difference of their significand lengths
	index_type expDiff = 0;
	if (!integerDifference(lhs.mExponent, rhs.mExponent, expDiff)) {
		// exp difference is too large, check which exponent has a larger size
		if (lhs.mExponent.isPositive != rhs.mExponent.isPositive) return (rhs.mExponent.isPositive == sign);
		bool expSign = lhs.mExponent.isPositive;
		index_type eL = lhs.mExponent.msb;
		index_type eR = rhs.mExponent.msb;
		while (eL > lhs.mExponent.lsb && lhs.mExponent.digits[eL] == 0) eL--;
		while (eR > rhs.mExponent.lsb && rhs.mExponent.digits[eR] == 0) eR--;
		index_type lExpSize = eL - lhs.mExponent.lsb + 1;
		index_type rExpSize = eR - rhs.mExponent.lsb + 1;
		bool rhsExpBigger = ((rExpSize > lExpSize) == expSign);
		return (rhsExpBigger == sign);
	}
	index_type sigSizeDiff = lSigSize - rSigSize;
	index_type magnitudeDiff = sigSizeDiff + expDiff;	// 0 if sigSizeDiff = -expDiff
	if (magnitudeDiff < 0) return sign;			// lhs has a smaller order of magnitude
	else if (magnitudeDiff > 0)	return !sign;	// rhs has a smaller order of magnitude

	// Check all significand digits
	for (; l >= lhs.mSignificand.lsb && r >= rhs.mSignificand.lsb; l--, r--) {
		if (lhs.mSignificand.digits[l] < rhs.mSignificand.digits[r]) return sign;
		else if (rhs.mSignificand.digits[r] < lhs.mSignificand.digits[l]) return !sign;
	}
	index_type lRep = lhs.mRepMSB;
	index_type rRep = rhs.mRepMSB;
	while (l >= lhs.mSignificand.lsb) {	// check the remaining lhs digits
		digit_type rDigit = 0;
		if (rRep >= 0) {
			rDigit = rhs.mSignificand.digits[rRep];
			if (--rRep < rhs.mRepLSB) rRep = rhs.mRepMSB;
		}
		if (lhs.mSignificand.digits[l] < rDigit) return sign;
		else if (rDigit < lhs.mSignificand.digits[l]) return !sign;
		l--;
	}
	while (r >= rhs.mSignificand.lsb) {	// check the remaining rhs digits
		digit_type lDigit = 0;
		if (lRep >= 0) {
			lDigit = lhs.mSignificand.digits[lRep];
			if (--lRep < lhs.mRepLSB) lRep = lhs.mRepMSB;
		}
		if (lDigit < rhs.mSignificand.digits[r]) return sign;
		else if (rhs.mSignificand.digits[r] < lDigit) return !sign;
		r--;
	}

	// Check if either significand has a repetend
	if (lhs.mRepLSB >= 0 || rhs.mRepLSB >= 0) {
		if (lhs.mRepLSB >= 0 && rhs.mRepLSB < 0) {			// only lhs has a repetend
			for (lRep = lhs.mRepMSB; lRep >= lhs.mRepLSB; lRep--)
				if (lhs.mSignificand.digits[lRep] != 0) return sign;
		} else if (rhs.mRepLSB >= 0 && lhs.mRepLSB < 0) {	// only rhs has a repetend
			for (rRep = rhs.mRepMSB; rRep >= rhs.mRepLSB; rRep--)
				if (rhs.mSignificand.digits[rRep] != 0) return !sign;
		} else {											// both have a repetend
			index_type lRepSize = lhs.mRepMSB - lhs.mRepLSB + 1;
			index_type rRepSize = rhs.mRepMSB - rhs.mRepLSB + 1;

			// line up the repetends if the significands are different lengths
			index_type lRepStart = lhs.mRepMSB, rRepStart = rhs.mRepMSB;
			index_type lSigSize = lhs.mSignificand.msb - lhs.mSignificand.lsb + 1;
			index_type rSigSize = rhs.mSignificand.msb - rhs.mSignificand.lsb + 1;
			if (lSigSize > rSigSize) rRepStart -= (lSigSize-rSigSize) % rRepSize;
			else if (rSigSize > lSigSize) lRepStart -= (rSigSize-lSigSize) % lRepSize;

			// extend each number's repetend, the most extra digits you need to compare is the lcm(lRepSize, rRepSize)
			index_type numExtraDigits = std::lcm(lRepSize, rRepSize);
			lRep = lRepStart;
			rRep = rRepStart;
			for (index_type i = 0; i < numExtraDigits; i++) {
				if (lhs.mSignificand.digits[lRep] < rhs.mSignificand.digits[rRep]) return sign;
				else if (rhs.mSignificand.digits[rRep] < lhs.mSignificand.digits[lRep]) return !sign;
				if (--lRep < lhs.mRepLSB) lRep = lhs.mRepMSB;
				if (--rRep < rhs.mRepLSB) rRep = rhs.mRepMSB;
			}
		}
	}

	return false;	// numbers are equal
}
bool bigNum::operator!=(const bigNum &rhs) const {
	return !(*this == rhs);
}
bool bigNum::operator>(const bigNum &rhs) const {
	return (rhs < *this);
}
bool bigNum::operator<=(const bigNum &rhs) const {
	return !(*this < rhs);
}
bool bigNum::operator>=(const bigNum &rhs) const {
	return !(*this > rhs);
}

// Newton-Raphson division
bigNum bigNum::newtonIter(const bigNum &x, const bigNum &d) {
	// Xi+1 = Xi + Xi*(1 - D*Xi)

	bigNum result(x.mSignificand.size, x.mExponent.size);
	result.mSignificand[0] = 1;
	bigNum temp(x);
	temp *= d;		// D*Xi
	result -= temp;	// 1 - D*Xi
	result *= x;	// Xi*(1 - D*Xi)
	result += x;	// Xi + Xi*(1 - D*Xi)

	return result;
}
bool bigNum::newtonDivide(const bigNum &rhs, size_type precision) {

	bigNum &lhs = *this;

	// Check for division by zero and bad precision
	if (rhs.mSignificand.isZero()) throw std::invalid_argument("Division by zero");
	if (precision == 0 || static_cast<index_type>(precision)+2 > mMaxDigits) throw std::invalid_argument("Invalid precision in newtonDivide");
	
	// Make an initial guess for x, the reciprocal of the divisor (1/rhs)
	// x0 (significand) = floor(10/rhs[msb] % 10) * 10^(precision+1)
	// x0 (exponent) = rhs exponent
	size_type xNumDigits = precision+2;
	size_type sigSize = std::max(xNumDigits, rhs.mSignificand.size);
	bigNum prevX(sigSize, rhs.mExponent.size);
	prevX.mSignificand.msb = xNumDigits-1;
	prevX.mSignificand.digits[prevX.mSignificand.msb] = 10/rhs.mSignificand.digits[rhs.mSignificand.msb] % 10;
	prevX.mSignificand.isPositive = rhs.mSignificand.isPositive;
	prevX.mExponent = rhs.mExponent;
	index_type rhsNumDigits = rhs.mSignificand.msb - rhs.mSignificand.lsb + 1;
	prevX.addExponent(rhsNumDigits+xNumDigits-1);
	prevX.mExponent.isPositive = !prevX.mExponent.isPositive;

	// Refine the guess using the Newton iteration until the desired precision is achieved
	size_type currP = mPrecision;
	mPrecision = xNumDigits;
	auto equal = [](const bigNum &x, const bigNum &y) -> bool {
		for (index_type i = 0; i <= x.mSignificand.msb; i++)
			if (x.mSignificand.digits[i] != y.mSignificand.digits[i]) return false;
		return true;
	};
	bigNum x = newtonIter(prevX, rhs);
	index_type count = 0, maxIter = 100;
	while (!equal(x, prevX) && count++ < maxIter) {
		prevX = x;
		x = newtonIter(x, rhs);
		std::cout << count << "\n";
	}
	if (count == maxIter && !equal(x, prevX)) {
		std::cerr << "Newton-Raphson division failed to converge";
		return false;
	}
	x.round(xNumDigits-1, true);

	// Multiply the numerator (this) by the reciprocal and round to get the quotient
	mPrecision = xNumDigits-1;
	lhs *= x;
	lhs.round(precision, false);
	mPrecision = currP;
	return true;
}

// Factorial - Using the "Swing Simple" algorithm given on http://www.luschny.de/math/factorial/index.html
bigNum factorialSwing(int64_t n) {
	int64_t z = 1;
	switch (n % 4) {
		case 1: z = n/2 + 1;		break;
		case 2: z = 2;				break;
		case 3: z = 2 * (n/2 + 2);	break;
		default: break;
	}
	bigNum b = z;
	z = 2 * (n - ((n+1) & 1));
	for (int64_t i = 1; i <= n/4; i++, z -= 4)
		b = (b * z) / i;
	return b;
}
bigNum factorialRec(int64_t n) {
	if (n < 2) return bigNum(1);
	bigNum b = factorialRec(n/2);
	b *= b;
	b *= factorialSwing(n);
	return b;	// factorialRec(n/2)^2 * factorialSwing(n)
}
bigNum bigNum::factorial(index_type n) {

	if (n < 0) throw std::invalid_argument("Factorial must be of a non-negative integer");

	index_type currP = mPrecision;
	bigNum::unsetPrecision();
	bigNum result = factorialRec(n);
	bigNum::setPrecision(currP);

	if (mPrecision) result.round(mPrecision, true);

	return result;
}
bigNum bigNum::factorial(const bigNum &num) {

	// Convert x to a C++ integer type
	index_type n = 0;
	if (!num.toInteger(n))
		throw std::invalid_argument("Factorial must be of a non-negative integer");

	return factorial(n);
}

// Arithmetic
bigNum& bigNum::operator+=(const bigNum &rhs) {

	// Find the difference between the lhs and rhs exponents
	index_type expDiff = 0;
	if (!integerDifference(mExponent, rhs.mExponent, expDiff))
		throw std::overflow_error("Overflow in operator+=");

	// The sum takes the smaller exponent
	bool expIsLarger = (mExponent.isPositive == rhs.mExponent.isPositive) ? (expDiff > 0) : mExponent.isPositive;
	if (expIsLarger) mExponent = rhs.mExponent;

	// Add the significands
	index_type sumShift = mSignificand.addSubtract(rhs.mSignificand, -expDiff, true, mPrecision);

	// Check for zero and shift
	if (mSignificand.isZero())
		mExponent.clear();
	else if (sumShift)
		addExponent(sumShift);

	return *this;
}
bigNum& bigNum::operator-=(const bigNum &rhs) {

	// Find the difference between the lhs and rhs exponents
	index_type expDiff = 0;
	if (!integerDifference(mExponent, rhs.mExponent, expDiff))
		throw std::overflow_error("Overflow in operator-=");

	// The difference takes the smaller exponent
	bool expIsLarger = (mExponent.isPositive == rhs.mExponent.isPositive) ? (expDiff > 0) : mExponent.isPositive;
	if (expIsLarger) mExponent = rhs.mExponent;

	// Subtract the significands
	index_type diffShift = mSignificand.addSubtract(rhs.mSignificand, -expDiff, false, mPrecision);
	
	// Check for zero and shift
	if (mSignificand.isZero())
		mExponent.clear();
	else if (diffShift)
		addExponent(diffShift);

	return *this;
}
bigNum& bigNum::operator*=(const bigNum &rhs) {

	// Sum the exponents
	mExponent.addSubtract(rhs.mExponent, 0, true, 0);

	// Multiply the significands
	index_type numDigits = mSignificand.msb - mSignificand.lsb + 1;
	index_type rhsNumDigits = rhs.mSignificand.msb - rhs.mSignificand.lsb + 1;
	index_type prodShift = 0;
	if (numDigits < karatsubaMinDigits && rhsNumDigits < karatsubaMinDigits)
		prodShift = mSignificand.longMultiply(rhs.mSignificand, mPrecision);
	else
		prodShift = mSignificand.karatsubaMultiply(rhs.mSignificand, mPrecision);

	// Check for zero and shift
	if (mSignificand.isZero())
		mExponent.clear();
	else if (prodShift)
		addExponent(prodShift);

	return *this;
}
bigNum& bigNum::operator/=(const bigNum &rhs) {

	// Find the difference between the lhs and rhs exponents
	index_type expDiff = 0;
	if (!integerDifference(mExponent, rhs.mExponent, expDiff))
		throw std::overflow_error("Overflow in operator/=");

	// Subtract the exponents
	mExponent.addSubtract(rhs.mExponent, 0, false, 0);

	// Divide the significands
	div_mode mode = mPrecision ? div_mode::PRECISION : div_mode::DEFAULT;
	index_type quotShift = mSignificand.longDivide(rhs.mSignificand, -expDiff, mode, mPrecision);

	// Check for zero and shift
	if (mSignificand.isZero())
		mExponent.clear();
	else if (quotShift)
		addExponent(quotShift);

	return *this;
}
bigNum& bigNum::operator%=(const bigNum &rhs) {
	// Remainder calculated using truncated division (to match C++11)
	bigNum q = euclideanDivide(*this, rhs);
	q *= rhs;
	*this -= q;
	return *this;
}
bigNum& bigNum::operator^=(const bigNum &rhs) {

	// Convert the rhs to a C++ integer type
	index_type exp = 0;
	if (!rhs.toInteger(exp))
		throw std::invalid_argument("Non-integer exponent in operator^=");

	return this->operator^=(exp);
}
bigNum& bigNum::operator^=(index_type rhs) {
	
	// Multiply the exponent by the rhs
	multiplyExponent(rhs);

	// Raise the significand to the rhs
	index_type powerShift = mSignificand.integerPow(rhs, mPrecision);

	// Check for zero and shift
	if (mSignificand.isZero())
		mExponent.clear();
	else if (powerShift)
		addExponent(powerShift);

	return *this;
}
bigNum bigNum::euclideanDivide(bigNum lhs, const bigNum &rhs) {

	// Find the difference between the lhs and rhs exponents
	bigNum::index_type expDiff = 0;
	if (!integerDifference(lhs.mExponent, rhs.mExponent, expDiff)) throw std::overflow_error("Overflow in euclideanDivide");

	// Subtract the exponents
	lhs.mExponent.addSubtract(rhs.mExponent, 0, false, 0);

	// Divide the significands
	index_type quotShift = lhs.mSignificand.longDivide(rhs.mSignificand, -expDiff, div_mode::EUCLID);

	// Check for zero and shift
	if (lhs.mSignificand.isZero())
		lhs.mExponent.clear();
	else if (quotShift)
		lhs.addExponent(-quotShift);

	return lhs;
}


bigNum operator+(bigNum lhs, const bigNum &rhs) {
	lhs += rhs;
	return lhs;
}
bigNum operator-(bigNum lhs, const bigNum &rhs) {
	lhs -= rhs;
	return lhs;
}
bigNum operator*(bigNum lhs, const bigNum &rhs) {
	lhs *= rhs;
	return lhs;
}
bigNum operator/(bigNum lhs, const bigNum &rhs) {
	lhs /= rhs;
	return lhs;
}
bigNum operator%(bigNum lhs, const bigNum &rhs) {
	lhs %= rhs;
	return lhs;
}
bigNum operator^(bigNum lhs, const bigNum &rhs) {
	lhs ^= rhs;
	return lhs;
}
bigNum operator^(bigNum lhs, int64_t rhs) {
	lhs ^= rhs;
	return lhs;
}








/*
bigNum::index_type bigNum::num_type::longDivide(div_mode mode, num_type rhs, index_type rhsShift) {

	// Check for division by zero and bad precision
	index_type z = rhs.msb;
	while (rhs.digits[z] == 0 && z > rhs.lsb) z--;
	if (z == rhs.lsb && rhs.digits[z] == 0) throw std::invalid_argument("Division by zero");
	if (mode == div_mode::PRECISION && mPrecision == 0) throw std::invalid_argument("Invalid precision in longDivide");

	// Helper function for single digit multiplication
	auto multiplyByDigit = [](num_type &num, digit_type d) {
		if (d == 1) return;
		digit_type carry = 0;
		for (index_type i = num.lsb; i <= num.msb; i++) {
			num.digits[i] = num.digits[i] * d + carry;
			carry = num.digits[i] / 10;
			num.digits[i] %= 10;
		}
		if (carry) {
			if (num.msb+1 >= num.size) num.resize(static_cast<index_type>(num.size)+1);
			num.digits[num.msb+1] = carry;
			num.msb++;
		}
	};

	// Return val, the shift required to make the quotient an integer
	index_type lhsShift = 0;
	num_type &lhs = *this;

	// Calculate the number of integer digits in the quotient (using rhsShift)
	index_type lhsNumDigits = lhs.msb - lhs.lsb + 1;
	index_type rhsNumDigits = rhs.msb - rhs.lsb + 1;
	index_type quotNumIntegerDigits = lhsNumDigits - (rhsNumDigits + rhsShift) + 1;
	if (quotNumIntegerDigits < 0) quotNumIntegerDigits = 0;	// lhs < rhs

	// Add/remove additional digits of precision
	index_type quotNumDigits = quotNumIntegerDigits;
	if (mode == div_mode::DEFAULT) quotNumDigits += (mDefaultDecimalPrecision + 1);
	else if (mode == div_mode::PRECISION) quotNumDigits = static_cast<index_type>(mPrecision) + 1;
	if (quotNumDigits > mMaxDigits) throw std::overflow_error("Overflow in longDivide");

	if (rhsNumDigits == 1) {	// If the divisor is a single digit, use a simpler algorithm
		digit_type d = rhs.digits[rhs.lsb];
		lhsShift = longDivideDigit(mode, d, quotNumIntegerDigits);
		isPositive = (isPositive == rhs.isPositive);
		return lhsShift;
	}

	// Allocate memory for the quotient
	size_type quotSize = (quotNumDigits > size) ? static_cast<size_type>(quotNumDigits) : size;
	digit_type *quotient = new digit_type[quotSize]();


	// Normalize lhs and rhs so that the first digit of rhs is >= 5 and lhs has an additional digit
	digit_type d = 10 / (rhs.digits[rhs.msb] + 1);	// normalization factor
	index_type lMSB = lhs.msb;
	multiplyByDigit(lhs, d);
	multiplyByDigit(rhs, d);
	if (lhs.msb == lMSB) {	// add a zero to the lhs
		if (lhs.msb+1 >= lhs.size) lhs.resize(static_cast<index_type>(lhs.size)+1);
		lhs.digits[lhs.msb+1] = 0;
		lhs.msb++;
	}

	// Allocate and initialize the partial dividend
	index_type n = rhsNumDigits + 1;	// size of partial dividend
	if (n > mMaxDigits) throw std::overflow_error("Overflow in longDivide");
	digit_type *div = new digit_type[static_cast<size_type>(n)]();
	index_type l = lhs.msb;
	for (index_type i = n-1; i >= 0 && l >= lhs.lsb; i--, l--)
		div[i] = lhs.digits[l];

	// Cache the products of the multiplication of the divisor by each digit
	num_type *products[10]{};

	// Divide u/v according to div_mode:
	// 		DEFAULT: divide to get an integer quotient then divide to the default decimal precision, stopping if the quotient terminates or repeats
	//		PRECISION: divide to the precision specified in mPrecision
	//		EUCLID: divide to get an integer quotient
	digit_type *u = lhs.digits;
	digit_type *v = rhs.digits;
	index_type p = 0;				// count number of precise digits
	index_type q = quotNumDigits-1;	// quotient index

	for (index_type i = 0; i < quotNumDigits && q >= 0; i++, q--) {
		// Divide the partial dividend (div) by the divisor (v) to get one digit (qHat) of the quotient

		// Calculate qHat
		digit_type qHat = (10*div[n-1]+div[n-2]) / v[rhs.msb];
		digit_type rHat = (10*div[n-1]+div[n-2]) % v[rhs.msb];
		while (qHat >= 10 || qHat*v[rhs.msb-1] >(10*rHat+div[n-3])) {
			qHat--;
			rHat += v[rhs.msb];
			if (rHat >= 10) break;
		}

		// Multiply v by qHat and subtract from the partial dividend
		if (!products[qHat]) {
			products[qHat] = new num_type(rhs);
			multiplyByDigit(*products[qHat], qHat);
		}
		num_type &product = *products[qHat];
		digit_type carry = 0;
		for (index_type j = 0, k = product.lsb; j < n; j++, k++) {
			div[j] = (9 - div[j]) + product.digits[k] + carry;
			if (div[j] > 9) {
				carry = 1;
				div[j] -= 10;
			} else carry = 0;
		}
		if (carry) {	// result is negative - q was 1 too large
			qHat--;
			// "End-around carry" - add the carry to the LSB to get the difference (a negative value)
			for (index_type j = 0; j < n; j++) {
				div[j] += carry;
				if (div[j] > 9) div[j] -= 10;
				else break;
			}
			// Add back v to get the correct partial quotient
			carry = 0;
			for (index_type j = 0, k = rhs.lsb; j < n-1; j++, k++) {
				div[j] += (9 - v[k]) + carry;
				if (div[j] > 9) {
					carry = 1;
					div[j] -= 10;
				} else carry = 0;
			}
			div[n-1] = 9;	// carry will always be 0 for the last digit
		}
		for (index_type j = 0; j < n; j++)
			div[j] = 9 - div[j];

		// Set this digit of the quotient
		quotient[q] = qHat;

		// Check for termination
		if (mode == div_mode::DEFAULT) {
			if (i >= quotNumIntegerDigits) {	// start counting precision after integer digits
				// Check for a zero remainder
				bool zeroRem = true;
				index_type j = 0;
				while (zeroRem && j < n) if (div[j++] != 0) zeroRem = false;
				if (zeroRem) { if (qHat == 0) q++; break; }
				if (p > 0 || qHat || quotNumIntegerDigits > 0) p++;
				if (p >= mDefaultDecimalPrecision) break;
			}
		} else if (mode == div_mode::PRECISION) {
			if (p > 0 || qHat) p++;
			if (p >= mPrecision) break;
		}

		// Shift the partial dividend
		for (index_type j = n-1; j > 0; j--)
			div[j] = div[j-1];
		div[0] = (l >= lhs.lsb) ? u[l--] : 0;
	}

	// Set the result
	delete[] div;
	for (index_type i = 0; i < 10; i++) delete products[i];



	//// Divide rhs into lhs and store the result in quotient
	//index_type qLSB = lhs.divideDigits(mode, rhs, quotient, quotNumDigits, quotNumIntegerDigits);

	// Set the result
	if (qLSB < 0) qLSB = 0;
	index_type numDigitsSet = quotNumDigits - qLSB;
	index_type numIntDigits = lhsNumDigits - rhsNumDigits + 1;	// ignoring rhsShift
	lhsShift = numDigitsSet - numIntDigits;
	switch (mode) {
		case div_mode::EUCLID:
			if (quotNumIntegerDigits <= 0) lhsShift = 0;
			[[fallthrough]];
		case div_mode::DEFAULT:
		case div_mode::PRECISION:
			delete[] digits;
			digits = quotient;
			size = quotSize;
			lsb = qLSB;
			msb = quotNumDigits ? (quotNumDigits-1) : 0;
			isPositive = (isPositive == rhs.isPositive);

			// Shift lsb to index 0
			while (digits[msb] == 0 && msb > lsb) msb--;
			if (lsb > 0) {
				index_type i = 0;
				for (index_type j = lsb; i < size && j <= msb && j < size; i++, j++)
					digits[i] = digits[j];
				while (i <= msb) digits[i++] = 0;
				msb -= lsb;
				lsb = 0;
			}
			break;
		default:
			delete[] quotient;
			lhsShift = 0;
	}

	return lhsShift;
}
*/


/*
bigNum::index_type bigNum::num_type::longDivideDigit(div_mode mode, digit_type d, index_type quotNumIntegerDigits) {

	if (d == 0) throw std::invalid_argument("Division by zero");
	//if (d == 1) return 0;

	// Initialize the quotient
	if (quotNumIntegerDigits < 0) quotNumIntegerDigits = 0;
	index_type quotNumDigits = quotNumIntegerDigits;
	if (mode == div_mode::DEFAULT) quotNumDigits += 9;
	else if (mode == div_mode::PRECISION) quotNumDigits = static_cast<index_type>(mPrecision) + 1;
	if (quotNumDigits > mMaxDigits) throw std::overflow_error("Overflow in longDivideDigit");
	size_type quotSize = (quotNumDigits > size) ? static_cast<size_type>(quotNumDigits) : size;
	digit_type *quotient = new digit_type[quotSize]();

	// Divide: this / d = quotient
	index_type q = quotNumDigits-1;
	index_type p = 0;				// count number of precise digits
	digit_type r = 0, qDigit = 0;
	for (index_type i = 0, j = msb; i < quotNumDigits && q >= 0; i++, j--, q--) {
		digit_type div = 10*r + (j >= lsb ? digits[j] : 0);	// 2 digit partial dividend
		qDigit = div / d;
		r = div % d;
		quotient[q] = qDigit;

		// Check for termination
		if (mode == div_mode::DEFAULT) {
			if (i >= quotNumIntegerDigits) {	// start counting precision after integer digits
				// Check for a zero remainder
				if (r == 0) {
					if (qDigit == 0) q++;
					break;
				}
				if (p > 0 || qDigit || quotNumIntegerDigits > 0) p++;
				if (p >= 9) break;
			}
		} else if (mode == div_mode::PRECISION) {
			if (p > 0 || qDigit) p++;
			if (p >= mPrecision) break;
		}
	}

	// Set the result
	if (q < 0) q = 0;
	index_type numDigitsSet = quotNumDigits - q;
	index_type numIntDigits = msb - lsb + 1;
	index_type lhsShift = numDigitsSet - numIntDigits;
	switch (mode) {
		case div_mode::EUCLID:
			if (quotNumIntegerDigits <= 0) lhsShift = 0;
			[[fallthrough]];
		case div_mode::DEFAULT:
		case div_mode::PRECISION:
			delete[] digits;
			digits = quotient;
			size = quotSize;
			lsb = q;
			msb = quotNumDigits ? (quotNumDigits-1) : 0;

			// Shift lsb to index 0
			while (digits[msb] == 0 && msb > lsb) msb--;
			if (lsb > 0) {
				index_type i = 0;
				for (index_type j = lsb; j <= msb && i < size; i++, j++)
					digits[i] = digits[j];
				while (i <= msb) digits[i++] = 0;
				msb -= lsb;
				lsb = 0;
			}
			break;
		default:
			delete[] quotient;
			lhsShift = 0;
	}

	return lhsShift;
}
*/


/*
bigNum::index_type bigNum::num_type::add(const num_type &rhs, index_type rhsShift, size_type precision) {

	// Calculate the number of digits needed for the sum
	index_type lhsNumDigits = msb - lsb + 1;
	index_type rhsNumDigits = rhs.msb - rhs.lsb + 1 + rhsShift;	// # of integer digits
	if (rhsNumDigits < 1) rhsNumDigits = 1;
	index_type sumNumDigits = std::max(lhsNumDigits, rhsNumDigits);
	if (rhsShift < 0) sumNumDigits += -rhsShift;	// if rhs is shifted right, sum has fraction digits
	if (sumNumDigits+1 > mMaxDigits) throw std::overflow_error("Overflow in add");

	// Allocate new memory for the sum if
	//	(1) there is not enough space or
	//	(2) calculation of the sum in-place will overwrite later digits (when rhs is shifted right or lhs is shifted left)
	digit_type *sum = digits;
	size_type sumSize = std::max(static_cast<size_type>(sumNumDigits)+1, size);
	if (precision > sumSize) sumSize = precision;
	index_type lhsShift = precision ? sumNumDigits-precision : 0;
	if (sumSize > size || rhsShift < 0 || (lhsShift < 0 && -lhsShift <= msb))
		sum = new digit_type[sumSize]();

	// Adjust the start index of the number with the larger lsb to account for the rhs shift
	const num_type &lhs = *this;
	index_type lhsStart = lhs.lsb - (rhsShift < 0 ? -rhsShift : 0);
	index_type rhsStart = rhs.lsb - (rhsShift > 0 ? rhsShift : 0);

	// Calculate the sum
	index_type sumMSB = precision ? precision-1 : sumNumDigits-1;		// <-----------------------remove this
	if (lhs.isPositive == rhs.isPositive) {
		// Add the digits
		digit_type carry = 0;
		for (index_type i = 0, l = lhsStart, r = rhsStart; i < sumNumDigits; i++, l++, r++) {
			digit_type left = (l < lhs.lsb || l > lhs.msb) ? 0 : lhs.digits[l];
			digit_type right = (r < rhs.lsb || r > rhs.msb) ? 0 : rhs.digits[r];
			digit_type d = left + right + carry;
			if (d > 9) {
				d -= 10;
				carry = 1;
			} else carry = 0;
			if (i >= lhsShift) sum[i-lhsShift] = d;
		}
		if (carry) {	// addition resulted in an additional digit
			if (precision) {
				for (index_type j = (lhsShift < 0 ? -lhsShift-1 : 0); j < precision-1; j++)
					sum[j] = sum[j+1];
				sum[precision-1] = carry;
				lhsShift++;
			} else {
				sumMSB++;
				sum[sumMSB] = carry;
			}
		}
	} else {
		// Subtract the digits
		digit_type carry = 0;
		for (index_type i = 0, l = lhsStart, r = rhsStart; i < sumNumDigits; i++, l++, r++) {
			digit_type left = (l < lhs.lsb || l > lhs.msb) ? 0 : lhs.digits[l];
			digit_type right = (r < rhs.lsb || r > rhs.msb) ? 0 : rhs.digits[r];
			digit_type d = (9 - left) + right + carry;
			if (d > 9) {
				d -= 10;
				carry = 1;
			} else carry = 0;
			//if (i >= lhsShift) sum[i-lhsShift] = d;	// no
			sum[i] = d;
		}
		if (carry) {	// result of subtraction is negative, |lhs| < |rhs|
			isPositive = !isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			index_type i = 0;
			while (carry && i < sumNumDigits) {
				sum[i] += carry;
				if (sum[i] > 9) sum[i] -= 10;
				else carry = 0;
				i++;
			}
		} else {		// result of subtraction is positive, |lhs| >= |rhs|
			for (index_type i = 0; i < sumNumDigits; i++)
				sum[i] = 9 - sum[i];
		}
		while (sum[sumMSB] == 0 && sumMSB > 0) sumMSB--;
		if (precision) {
			lhsShift -= (sumNumDigits-1-sumMSB);
		}
	}

	// Set the result
	if (sum == digits) {	// no new data was was allocated, clear any unchanged digits
		if (lhsShift < 0) {
			index_type l = lsb;
			while (l < -lhsShift) sum[l++] = 0;
		}
		index_type m = msb;
		while (m > sumMSB) digits[m--] = 0;
	} else {				// new data was allocated
		delete[] digits;
		digits = sum;
		size = sumSize;
	}
	lsb = 0;
	msb = precision ? precision-1 : sumNumDigits-1;
	while (sum[msb] == 0 && msb > lsb) msb--;

	//index_type lhsShift = 0;	// the change in the number's size as a result of the addition
	//sumNumDigits = sumMSB + 1;
	//lhsShift = sumNumDigits - lhsNumDigits;

	return lhsShift;
}
bigNum::index_type bigNum::num_type::subtract(const num_type &rhs, index_type rhsShift, size_type precision) {
	// Calculate the number of digits needed for the diff
	index_type numDigits = msb - lsb + 1;						// # of integer digits
	index_type rhsNumDigits = rhs.msb - rhs.lsb + 1 + rhsShift;	// # of integer digits
	if (rhsNumDigits < 1) rhsNumDigits = 1;
	index_type diffNumDigits = (numDigits > rhsNumDigits ? numDigits : rhsNumDigits);
	if (rhsShift < 0) diffNumDigits += -rhsShift;	// if rhs is shifted right, diff has fraction digits
	if (diffNumDigits+1 > mMaxDigits) throw std::overflow_error("Overflow in add");

	// If the rhs is shifted to the right or has a greater size, allocate new memory for the diff
	digit_type *diff = digits;
	size_type diffSize = (diffNumDigits+1 > size) ? static_cast<size_type>(diffNumDigits)+1 : size;
	if (rhsShift < 0 || diffSize > size)
		diff = new digit_type[diffSize]();

	// Adjust the start index of the number with the larger lsb to account for the rhs shift
	const num_type &lhs = *this;
	index_type lhsStart = lhs.lsb - (rhsShift < 0 ? -rhsShift : 0);
	index_type rhsStart = rhs.lsb - (rhsShift > 0 ? rhsShift : 0);

	// Calculate the difference
	index_type diffLSB = 0;
	index_type diffMSB = diffNumDigits-1;
	if (lhs.isPositive == rhs.isPositive) {
		// Subtract the digits
		for (index_type i = diffLSB, l = lhsStart; i <= diffMSB; i++, l++)
			diff[i] = 9 - (l < lhs.lsb || l > lhs.msb ? 0 : lhs.digits[l]);
		digit_type carry = 0;
		for (index_type i = diffLSB, r = rhsStart; i <= diffMSB; i++, r++) {
			diff[i] += (r < rhs.lsb || r > rhs.msb ? 0 : rhs.digits[r]) + carry;
			if (diff[i] > 9) {
				diff[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
		if (carry) {	// result of subtraction is negative, |lhs| < |rhs|
			isPositive = !isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			index_type i = diffLSB;
			while (carry && i <= diffMSB) {
				diff[i] += carry;
				if (diff[i] > 9) diff[i] -= 10;
				else carry = 0;
				i++;
			}
		} else {		// result of subtraction is positive, |lhs| >= |rhs|
			for (index_type i = diffLSB; i <= diffMSB; i++)
				diff[i] = 9 - diff[i];
		}

	} else {
		// Add the digits
		digit_type carry = 0;
		index_type i = diffLSB;
		for (index_type l = lhsStart, r = rhsStart; i <= diffMSB; i++, l++, r++) {
			digit_type left = (l < lhs.lsb || l > lhs.msb) ? 0 : lhs.digits[l];
			digit_type right = (r < rhs.lsb || r > rhs.msb) ? 0 : rhs.digits[r];
			diff[i] = left + right + carry;
			if (diff[i] > 9) {
				diff[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
		if (carry) {	// addition resulted in an additional digit
			diff[i] = carry;
			diffMSB = i;
		}
	}

	// Set the result
	if (diff == digits) {	// no new data was was allocated
		// clear any unchanged digits
		index_type i = msb, j = lsb;
		while (i > diffMSB) digits[i--] = 0;
		while (j < diffLSB) digits[j++] = 0;
	} else {				// new data was allocated
		delete[] digits;
		digits = diff;
		size = diffSize;
	}
	while (diff[diffMSB] == 0 && diffMSB > diffLSB) diffMSB--;
	lsb = diffLSB;
	msb = diffMSB;

	index_type lhsShift = 0;	// the change in the number's size as a result of the subtraction
	diffNumDigits = diffMSB - diffLSB + 1;
	lhsShift = diffNumDigits - numDigits;

	return lhsShift;
}
*/