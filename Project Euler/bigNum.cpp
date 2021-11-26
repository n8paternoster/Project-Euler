#include "stdafx.h"
#include "bigNum.h"
#include <string>
#include <limits>			// std::digits10
#include <numeric>			// std::lcm
#include <unordered_map>	// hash table to store remainders during division 

/* ------------------------------------------------------ */
/*                      num_type                          */
/* -------------------------------------------------------*/

// Special member functions
bigNum::num_type::num_type():
	digits(new digit_type[mDefaultSize]()), size(mDefaultSize), lsb(0), msb(0), isPositive(true)
{ }
bigNum::num_type::num_type(size_type sz):
	lsb(0), msb(0), isPositive(true)
{
	if (sz < 1) sz = mDefaultSize;
	size = sz;
	digits = new digit_type[size]();
}
bigNum::num_type::num_type(const num_type &other):
	digits(new digit_type[other.size]()), size(other.size), lsb(other.lsb), msb(other.msb), isPositive(other.isPositive)
{
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
bigNum::digit_type bigNum::num_type::operator[](index_type i) const{
	if (i > size || i < 0 || i > (msb - lsb)) throw std::range_error("Index out of range");
	return digits[msb-i];
}
bigNum::digit_type& bigNum::num_type::operator[](index_type i) {
	if (i > size || i < 0 || i >(msb - lsb)) throw std::range_error("Index out of range");
	return digits[msb-i];
}
bool bigNum::num_type::toInteger(index_type &out) const {
	index_type numDigits = msb - lsb + 1;
	constexpr index_type maxNumDigits = std::numeric_limits<index_type>::digits10;
	if (numDigits <= maxNumDigits) {
		out = 0;
		for (index_type i = lsb, e = 1; i <= msb && i < size; i++, e *= 10)
			out += e * digits[i];
		if (!isPositive) out = -out;
		return true;
	}
	return false;
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
	for (index_type i = 0; i < size && i < newSize; i++)
		newDigits[i] = digits[i];
	delete[] digits;
	size = newSize;
	digits = newDigits;
}

// Arithmetic
bigNum::index_type bigNum::num_type::add(const num_type &rhs, index_type rhsShift) {

	// Calculate the number of digits needed for the sum
	index_type numDigits = msb - lsb + 1;						// # of integer digits
	index_type rhsNumDigits = rhs.msb - rhs.lsb + 1 + rhsShift;	// # of integer digits
	if (rhsNumDigits < 1) rhsNumDigits = 1;
	index_type sumNumDigits = (numDigits > rhsNumDigits ? numDigits : rhsNumDigits);
	if (rhsShift < 0) sumNumDigits += -rhsShift;	// if rhs is shifted right, sum has fraction digits
	if (sumNumDigits+1 > mMaxDigits) throw std::overflow_error("Overflow in add");

	// If the rhs is shifted to the right or has a greater size, allocate new memory for the sum
	digit_type *sum = digits;
	size_type sumSize = (sumNumDigits+1 > size) ? static_cast<size_type>(sumNumDigits)+1 : size;
	if (rhsShift < 0 || sumSize > size)
		sum = new digit_type[sumSize]();

	// Adjust the start index of the number with the larger lsb to account for the rhs shift
	const num_type &lhs = *this;
	index_type lhsStart = lhs.lsb - (rhsShift < 0 ? -rhsShift : 0);
	index_type rhsStart = rhs.lsb - (rhsShift > 0 ? rhsShift : 0);

	// Calculate the sum
	index_type sumLSB = 0;
	index_type sumMSB = sumNumDigits-1;
	if (lhs.isPositive == rhs.isPositive) {
		// Add the digits
		digit_type carry = 0;
		index_type i = sumLSB;
		for (index_type l = lhsStart, r = rhsStart; i <= sumMSB; i++, l++, r++) {
			digit_type left = (l < lhs.lsb || l > lhs.msb) ? 0 : lhs.digits[l];
			digit_type right = (r < rhs.lsb || r > rhs.msb) ? 0 : rhs.digits[r];
			sum[i] = left + right + carry;
			if (sum[i] > 9) {
				sum[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
		if (carry) {	// addition resulted in an additional digit
			sum[i] = carry;
			sumMSB = i;
		}
	} else {
		// Subtract the digits
		for (index_type i = sumLSB, l = lhsStart; i <= sumMSB; i++, l++)
			sum[i] = 9 - (l < lhs.lsb || l > lhs.msb ? 0 : lhs.digits[l]);
		digit_type carry = 0;
		for (index_type i = sumLSB, r = rhsStart; i <= sumMSB; i++, r++) {
			sum[i] += (r < rhs.lsb || r > rhs.msb ? 0 : rhs.digits[r]) + carry;
			if (sum[i] > 9) {
				sum[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
		if (carry) {	// result of subtraction is negative, |lhs| < |rhs|
			isPositive = !isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			index_type i = sumLSB;
			while (carry && i <= sumMSB) {
				sum[i] += carry;
				if (sum[i] > 9) sum[i] -= 10;
				else carry = 0;
				i++;
			}
		} else {		// result of subtraction is positive, |lhs| >= |rhs|
			for (index_type i = sumLSB; i <= sumMSB; i++)
				sum[i] = 9 - sum[i];
		}
	}

	// Set the result
	if (sum == digits) {	// no new data was was allocated
		// clear any unchanged digits
		index_type i = msb, j = lsb;
		while (i > sumMSB) digits[i--] = 0;
		while (j < sumLSB) digits[j++] = 0;
	} else {				// new data was allocated
		delete[] digits;
		digits = sum;
		size = sumSize;
	}
	while (sum[sumMSB] == 0 && sumMSB > sumLSB) sumMSB--;
	lsb = sumLSB;
	msb = sumMSB;

	index_type lhsShift = 0;	// the change in the number's size as a result of the addition
	sumNumDigits = sumMSB - sumLSB + 1;
	lhsShift = sumNumDigits - numDigits;

	return lhsShift;
}
bigNum::index_type bigNum::num_type::subtract(const num_type &rhs, index_type rhsShift) {
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
void bigNum::num_type::longMultiply(const num_type &rhs) {
	
	// Allocate space for the product
	index_type numDigits = msb - lsb + 1;
	index_type rhsNumDigits = rhs.msb - rhs.lsb + 1;
	index_type prodNumDigits = numDigits + rhsNumDigits;
	if (prodNumDigits > mMaxDigits) throw std::overflow_error("Overflow in longMultiply");
	size_type prodSize = (prodNumDigits > size) ? static_cast<size_type>(prodNumDigits) : size;
	digit_type *prod = new digit_type[prodSize]();

	// Calculate the product
	index_type sum = 0;
	for (index_type p = 0; p < prodSize-1; p++) {	// for each digit of the product
		for (index_type r = (p < size) ? 0 : p-size+1; r <= p && r < rhs.size; r++)
			sum += digits[p-r] * rhs.digits[r];
		prod[p] = sum % 10;
		sum /= 10;
	}
	prod[prodSize-1] = sum % 10;

	// Set the result
	delete[] digits;
	digits = prod;
	size = prodSize;
	lsb = 0;
	msb = prodNumDigits-1;
	while (digits[msb] == 0 && msb > lsb) msb--;
	isPositive = (isPositive == rhs.isPositive);
}
bigNum::index_type bigNum::num_type::divideDigits(div_mode mode, num_type &rhs, digit_type *quotient, index_type quotNumDigits, index_type quotNumIntegerDigits) {
	// Divide this by rhs and store the result in quotient
	// Return the index of the quotient's least significant digit set
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
			if (mode == div_mode::default) {
				if (i >= quotNumIntegerDigits) {	// start counting precision after integer digits
					// Check for a zero remainder
					if (r == 0) {
						if (qDigit == 0) q++;
						break;
					}
					if (p > 0 || qDigit || quotNumIntegerDigits > 0) p++;
					if (p >= 9) break;
				}
			} else if (mode == div_mode::precision) {
				if (p > 0 || qDigit) p++;
				if (p >= mPrecision) break;
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
		if (mode == div_mode::default) {
			if (i >= quotNumIntegerDigits) {	// start counting precision after integer digits
				// Check for a zero remainder
				bool zeroRem = true;
				index_type j = 0;
				while (zeroRem && j < n) if (div[j++] != 0) zeroRem = false;
				if (zeroRem) { if (qHat == 0) q++; break; }
				if (p > 0 || qHat || quotNumIntegerDigits > 0) p++;
				if (p >= mDefaultDecimalPrecision) break;
			}
		} else if (mode == div_mode::precision) {
			if (p > 0 || qHat) p++;
			if (p >= mPrecision) break;
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
bigNum::index_type bigNum::num_type::longDivide(div_mode mode, num_type rhs, index_type rhsShift) {

	// Check for division by zero and bad precision
	index_type z = rhs.msb;
	while (rhs.digits[z] == 0 && z > rhs.lsb) z--;
	if (z == rhs.lsb && rhs.digits[z] == 0) throw std::invalid_argument("Division by zero");
	if (mode == div_mode::precision && mPrecision == 0) throw std::invalid_argument("Invalid precision in longDivide");

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
	if (mode == div_mode::default) quotNumDigits += (mDefaultDecimalPrecision + 1);
	else if (mode == div_mode::precision) quotNumDigits = static_cast<index_type>(mPrecision) + 1;
	if (quotNumDigits > mMaxDigits) throw std::overflow_error("Overflow in longDivide");

	// Allocate memory for the quotient
	size_type quotSize = (quotNumDigits > size) ? static_cast<size_type>(quotNumDigits) : size;
	digit_type *quotient = new digit_type[quotSize]();

	// Divide rhs into lhs and store the result in quotient
	index_type qLSB = lhs.divideDigits(mode, rhs, quotient, quotNumDigits, quotNumIntegerDigits);

	// Set the result
	if (qLSB < 0) qLSB = 0;
	index_type numDigitsSet = quotNumDigits - qLSB;
	index_type numIntDigits = lhsNumDigits - rhsNumDigits + 1;	// ignoring rhsShift
	lhsShift = numDigitsSet - numIntDigits;
	switch (mode) {
		case div_mode::euclid:
			if (quotNumIntegerDigits <= 0) lhsShift = 0;	
			[[fallthrough]];
		case div_mode::default:
		case div_mode::precision:
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


/* ------------------------------------------------------ */
/*					   Constructors                       */
/* -------------------------------------------------------*/

bigNum::bigNum():
	mSignificand(), mExponent(), mRepLSB(-1), mRepMSB(-1)
{ }
bigNum::bigNum(size_type sigSize, size_type expSize):
	mSignificand(sigSize), mExponent(expSize), mRepLSB(-1), mRepMSB(-1)
{ }
bigNum::bigNum(std::string num):
	mSignificand(), mExponent(), mRepLSB(-1), mRepMSB(-1)
{
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

/* ------------------------------------------------------ */
/*                       Utility                          */
/* -------------------------------------------------------*/

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

	constexpr index_type maxExpPrintSize = 1'000;

	// Get the value of the exponent, if it is too large, print in floating-point notation
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
			}
			else std::cout << "0";							// print extra 0's from scaling
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

	// Calculate the difference digit by digit
	num_type diff(lhs);
	diff.subtract(rhs);
	index_type diffInt = 0;
	if (!diff.toInteger(diffInt)) return false;
	out = diffInt;
	return true;
}
void bigNum::shift(index_type shiftAmt) {
	// add shiftAmt to mExponent

	if (shiftAmt == 0) return;
	bool shiftIsPositive = (shiftAmt > 0);
	if (shiftAmt < 0) shiftAmt = -shiftAmt;
	index_type numShiftDigits = static_cast<index_type>(std::log10(shiftAmt)) + 1;
	if (mExponent.lsb+numShiftDigits+1 > mExponent.size)
		mExponent.resize(mExponent.lsb+numShiftDigits+1);

	if (mExponent.isPositive == shiftIsPositive) {
		// add shiftAmt to mExponent
		index_type i = mExponent.lsb;
		digit_type carry = 0;
		while ((carry || shiftAmt) && i < mExponent.size) {
			if (i > mExponent.msb) mExponent.digits[i] = 0;
			mExponent.digits[i] += (shiftAmt % 10) + carry;
			if (mExponent.digits[i] > 9) {
				mExponent.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
			shiftAmt /= 10;
			i++;
		}
		if (i-1 > mExponent.msb) mExponent.msb = i-1;
	} else {
		// subtract shiftAmt from mExponent
		index_type numExpDigits = mExponent.msb - mExponent.lsb + 1;
		index_type numDigits = (numExpDigits > numShiftDigits) ? numExpDigits : numShiftDigits;
		for (index_type i = mExponent.lsb, j = 0; j < numDigits && i < mExponent.size; i++, j++) {
			if (i > mExponent.msb) mExponent.digits[i] = 0;
			mExponent.digits[i] = 9 - mExponent.digits[i];
		}
		digit_type carry = 0;
		for (index_type i = mExponent.lsb, j = 0; j < numDigits; i++, j++) {
			mExponent.digits[i] += (shiftAmt % 10) + carry;
			if (mExponent.digits[i] > 9) {
				mExponent.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
			shiftAmt /= 10;
		}
		if (carry) {	// result of subtraction is negative
			mExponent.isPositive = !mExponent.isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			index_type i = mExponent.lsb;
			while (i < mExponent.size) {
				mExponent.digits[i] += carry;
				if (mExponent.digits[i] > 9) mExponent.digits[i] -= 10;
				else break;
				i++;
			}
		} else {		// result of subtraction is positive
			for (index_type i = mExponent.lsb, j = 0; j < numDigits; i++, j++)
				mExponent.digits[i] = 9 - mExponent.digits[i];
		}
		mExponent.msb = mExponent.lsb + numDigits - 1;
		while (mExponent.digits[mExponent.msb] == 0 && mExponent.msb > mExponent.lsb) mExponent.msb--;
	}
	if (mExponent.msb == mExponent.lsb && mExponent.digits[mExponent.lsb] == 0) mExponent.isPositive = true;	 // check for -zero
}
std::ostream& operator<<(std::ostream &out, const bigNum &num) {
	num.print();
	return out;
}

/* ------------------------------------------------------ */
/*                      Comparison                        */
/* -------------------------------------------------------*/

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

/* ------------------------------------------------------ */
/*                      Arithmetic                        */
/* -------------------------------------------------------*/

// Karatsuba multiplication
std::pair<bigNum::num_type, bigNum::num_type> bigNum::num_type::splitAt(index_type m) const {
	index_type numDigits = msb - lsb + 1;
	if (m == 0) return {num_type(1), num_type(*this)};
	if (m > mMaxDigits) throw std::overflow_error("Overflow in splitAt");
	if (m >= numDigits) return {num_type(*this), num_type(1)};
	num_type low(static_cast<size_type>(m)), high(static_cast<size_type>(numDigits-m));
	index_type i = lsb;
	for (index_type l = 0; i < lsb+m && i < size; i++, l++)
		low.digits[l] = digits[i];
	for (index_type h = 0; i <= msb && i < size; i++, h++)	// digit at index belongs to high
		high.digits[h] = digits[i];
	low.lsb = 0;
	low.msb = m-1;
	high.lsb = 0;
	high.msb = numDigits-m-1;
	return {low, high};
}
bigNum::num_type bigNum::karatsuba(const num_type &a, const num_type &b) const {

	// Base case: For operands under 256 digits, use long multiplication
	index_type aNumDigits = a.msb - a.lsb + 1;
	index_type bNumDigits = b.msb - b.lsb + 1;
	if (aNumDigits < 256 && bNumDigits < 256) {
		num_type result(a);
		result.longMultiply(b);
		return result;
	}

	// Choose m, the position to split each number
	index_type n = (aNumDigits > bNumDigits) ? aNumDigits : bNumDigits;
	index_type m = n / 2;

	// Split a and b into smaller numbers with approximately half the number of digits
	auto [aLow, aHigh] = a.splitAt(m);
	auto [bLow, bHigh] = b.splitAt(m);

	// Recursive calls on numbers of approximately half size
	num_type z0 = karatsuba(aLow, bLow);
	num_type z2 = karatsuba(aHigh, bHigh);
	aLow.add(aHigh);
	bLow.add(bHigh);
	num_type z1 = karatsuba(aLow, bLow);
	z1.subtract(z0);
	z1.subtract(z2);

	z0.add(z1, m);		// multiply z1 by 10^m and add to z0
	z0.add(z2, 2*m);	// multiply z2 by 10^(2*m) and add to z0
	return z0;
}

bigNum& bigNum::operator+=(const bigNum &rhs) {
	
	// Find the difference between the lhs and rhs exponents
	index_type expDiff = 0;
	if (!integerDifference(mExponent, rhs.mExponent, expDiff)) throw std::overflow_error("Overflow in operator+=");

	// Add the significands
	index_type expShift = mSignificand.add(rhs.mSignificand, -expDiff);

	// The sum takes the smaller exponent
	bool expIsLarger = (mExponent.isPositive == rhs.mExponent.isPositive) ? (expDiff > 0) : mExponent.isPositive;
	if (expIsLarger) mExponent = rhs.mExponent;

	return *this;
}
bigNum& bigNum::operator-=(const bigNum &rhs) {

	// Find the difference between the lhs and rhs exponents
	index_type expDiff = 0;
	if (!integerDifference(mExponent, rhs.mExponent, expDiff)) throw std::overflow_error("Overflow in operator-=");

	// Set the significand
	index_type expShift = mSignificand.subtract(rhs.mSignificand, -expDiff);

	// Adjust the exponent
	bool expIsLarger = (mExponent.isPositive == rhs.mExponent.isPositive) ? (expDiff > 0) : mExponent.isPositive;
	if (expIsLarger) mExponent = rhs.mExponent;

	return *this;
}
bigNum& bigNum::operator*=(const bigNum &rhs) {

	// Sum the exponents
	mExponent.add(rhs.mExponent);

	// Multiply the significands
	index_type numDigits = mSignificand.msb - mSignificand.lsb + 1;
	index_type rhsNumDigits = rhs.mSignificand.msb - rhs.mSignificand.lsb + 1;
	if (numDigits < 256 && rhsNumDigits < 256) {		// long multiply
		mSignificand.longMultiply(rhs.mSignificand);	
	} else {											// karatsuba multiply
		mSignificand = karatsuba(mSignificand, rhs.mSignificand);
		mSignificand.isPositive = (mSignificand.isPositive == rhs.mSignificand.isPositive);
	}

	return *this;
}
bigNum& bigNum::operator/=(const bigNum &rhs) {

	// Find the difference between the lhs and rhs exponents
	index_type expDiff = 0;
	if (!integerDifference(mExponent, rhs.mExponent, expDiff)) throw std::overflow_error("Overflow in operator/=");

	// Subtract the exponents
	mExponent.subtract(rhs.mExponent);

	// Divide the significands
	div_mode mode = mPrecision ? div_mode::precision : div_mode::default;
	index_type shiftAmt = mSignificand.longDivide(mode, rhs.mSignificand, -expDiff);
	shift(-shiftAmt);	// shift to keep the significand an integer

	return *this;
}
bigNum& bigNum::operator%=(const bigNum &rhs) {
	return *this;
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











/*
bigNum::index_type bigNum::num_type::longDivide(div_mode mode, num_type rhs, index_type rhsShift) {

	// Check for division by zero and bad precision
	index_type z = rhs.msb;
	while (rhs.digits[z] == 0 && z > rhs.lsb) z--;
	if (z == rhs.lsb && rhs.digits[z] == 0) throw std::invalid_argument("Division by zero");
	if (mode == div_mode::precision && mPrecision == 0) throw std::invalid_argument("Invalid precision in longDivide");

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
	if (mode == div_mode::default) quotNumDigits += (mDefaultDecimalPrecision + 1);
	else if (mode == div_mode::precision) quotNumDigits = static_cast<index_type>(mPrecision) + 1;
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
		if (mode == div_mode::default) {
			if (i >= quotNumIntegerDigits) {	// start counting precision after integer digits
				// Check for a zero remainder
				bool zeroRem = true;
				index_type j = 0;
				while (zeroRem && j < n) if (div[j++] != 0) zeroRem = false;
				if (zeroRem) { if (qHat == 0) q++; break; }
				if (p > 0 || qHat || quotNumIntegerDigits > 0) p++;
				if (p >= mDefaultDecimalPrecision) break;
			}
		} else if (mode == div_mode::precision) {
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
		case div_mode::euclid:
			if (quotNumIntegerDigits <= 0) lhsShift = 0;
			[[fallthrough]];
		case div_mode::default:
		case div_mode::precision:
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
	if (mode == div_mode::default) quotNumDigits += 9;
	else if (mode == div_mode::precision) quotNumDigits = static_cast<index_type>(mPrecision) + 1;
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
		if (mode == div_mode::default) {
			if (i >= quotNumIntegerDigits) {	// start counting precision after integer digits
				// Check for a zero remainder
				if (r == 0) {
					if (qDigit == 0) q++;
					break;
				}
				if (p > 0 || qDigit || quotNumIntegerDigits > 0) p++;
				if (p >= 9) break;
			}
		} else if (mode == div_mode::precision) {
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
		case div_mode::euclid:
			if (quotNumIntegerDigits <= 0) lhsShift = 0;
			[[fallthrough]];
		case div_mode::default:
		case div_mode::precision:
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