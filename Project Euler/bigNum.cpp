#include "stdafx.h"
#include "bigNum.h"
#include <sstream>
#include <string>
#include <limits>
#include <unordered_map>	// to store remainders during division to check for repetends

namespace std {
	// specialize std::hash with the bigNum type
	template<> struct hash<bigNum> {
		size_t operator()(const bigNum& n) const {	// assumes no leading/trailing zeros
			hash<digitType> hasher;
			size_t h = 0;
			for (sizeType d = 0; d < n.mSize; d++)
				h = h * 31 + hasher(n.mDigits[d]);
			return h;
		}
	};
}

const sizeType bigNum::defaultPrecision = 10;

bool bigNum::updateState() { // Updates mNumDigits, mIsZero, mIsInteger
	if (mSize == 0) {
		mNumDigits = 0;			// should be 1 for the number 0?
		mIsNegative = false;
		mIsZero = true;
		mIsInteger = true;
		return false;
	}

	// Count the fraction digits
	sizeType numFractionDigits = 0;
	sizeType start = 0;
	while (start < mIntegerStart && (mRepetendSize > 0 ? (start < mRepetendStart) : 1) && mDigits[start] == 0) start++;
	numFractionDigits = mIntegerStart - start;

	// Count the integer digits
	sizeType numIntegerDigits = 0;
	sizeType end = mSize - 1;
	while (end > mIntegerStart && mDigits[end] == 0) end--;
	numIntegerDigits = end - mIntegerStart + 1;

	mNumDigits = numFractionDigits + numIntegerDigits;
	if (mNumDigits == 1 && mDigits[mIntegerStart] == 0) {
		mIsZero = true;
		mIsNegative = false;	// no representation for -0
	}  else mIsZero = false;
	mIsInteger = numFractionDigits > 0 ? false : true;

	return true;
}

void bigNum::setNegative() {
	mIsNegative = true;
}

void bigNum::setPositive() {
	mIsNegative = false;
}

// Removes leading, integer zeros
bool bigNum::removeLeadingZeros() { // Updates mSize, mDigits
	if (mSize == 0) return false;

	sizeType end = mSize - 1;
	while (end > mIntegerStart && mDigits[end] == 0) end--;
	if (end != (mSize - 1)) {
		sizeType newSize = end + 1;
		digitType *newData = new digitType[newSize];
		for (sizeType i = 0; i < newSize; i++)
			newData[i] = mDigits[i];
		delete[] mDigits;
		mDigits = newData;
		mSize = newSize;
		return true;
	}

	return false;
}

// Removes trailing, fractional zeros
bool bigNum::removeTrailingZeros() { // Updates mSize, mDigits, mIntegerStart
	if (mSize == 0) return false;

	sizeType start = 0;
	while (start < mIntegerStart && (mRepetendSize > 0 ? (start < mRepetendStart) : 1) && mDigits[start] == 0) start++;
	if (start != 0) {
		sizeType newSize = mSize - start;
		digitType *newData = new digitType[newSize];
		for (sizeType i = 0, j = start; i < newSize; i++, j++)
			newData[i] = mDigits[j];
		delete[] mDigits;
		mDigits = newData;
		mSize = newSize;
		mIntegerStart -= start;
		if (mRepetendSize > 0) mRepetendStart -= start;
		return true;
	}

	return false;
}

// New size must be > the number of fraction digits + 1
bool bigNum::resize(sizeType newSize) {
	if (newSize <= (mIntegerStart + 1)) return false;

	sizeType oldSize = mSize;
	digitType *newData = new digitType[newSize];

	// Copy the digits over, starting from the LSB
	sizeType i = 0;
	for (; i < oldSize; i++)
		newData[i] = mDigits[i];

	// Pad zeros beyond the MSB
	for (; i < newSize; i++)
		newData[i] = 0;

	delete[] mDigits;
	mDigits = newData;
	mSize = newSize;
	updateState();

	return true;
}

// Equivalent to multiplying the number by 10^shiftAmt
// Returns true if digits were added, false otherwise
bool bigNum::shiftLeft(sizeType shiftAmt) {		// updates mIntegerStart, mSize, mDigits, mNumDigits
	if (shiftAmt <= 0) return false;

	bool returnVal = false;

	// shift the position of the decimal point first
	if (mIntegerStart > 0) {
		sizeType radixShiftAmt = (mIntegerStart > shiftAmt) ? shiftAmt : mIntegerStart;
		mIntegerStart -= radixShiftAmt;
		shiftAmt -= radixShiftAmt;
	}

	// add additional digits
	if (shiftAmt > 0) {
		sizeType numNewDigits = shiftAmt + mRepetendSize;
		sizeType newSize = mSize + numNewDigits;
		digitType *newData = new digitType[newSize];
		sizeType i = 0;

		// add new digits
		if (mRepetendSize > 0) {	// extend the repetend
			for (sizeType r = mRepetendStart + (mRepetendSize-1); i < numNewDigits; i++) {
				sizeType index = (numNewDigits-1) - i;
				newData[index] = mDigits[r];
				if (r == mRepetendStart) r += mRepetendSize-1;
				else r--;
			}
		} else						// pad zeros
			for (; i < numNewDigits; i++)
				newData[i] = 0;

		// copy old digits
		for (sizeType j = 0; j < mSize; j++, i++)
			newData[i] = mDigits[j];

		// update members
		delete[] mDigits;
		mDigits = newData;
		mSize = newSize;
		if (mRepetendSize > 0) {	// additional fraction digits were added for the repetend
			mRepetendStart = 0;
			mIntegerStart += mRepetendSize;
		}

		returnVal = true;
	} else if (mRepetendSize > 0 && (mRepetendStart + mRepetendSize) > mIntegerStart) {	

		// repetend contains integer digits, adjust so it is entirely fractional

		// adjust the beginning of the repetend
		while (mRepetendStart > 0 && (mRepetendStart + mRepetendSize) > mIntegerStart) mRepetendStart--;

		// add additional digits
		if ((mRepetendStart + mRepetendSize) > mIntegerStart) {
			sizeType numNewDigits = (mRepetendStart + mRepetendSize) - mIntegerStart;
			sizeType newSize = mSize + numNewDigits;
			digitType *newData = new digitType[newSize];
			sizeType i = 0;

			// add new digits
			for (sizeType r = mRepetendStart + (mRepetendSize-1); i < numNewDigits; i++) {
				sizeType index = (numNewDigits-1) - i;
				newData[index] = mDigits[r];
				if (r == mRepetendStart) r += mRepetendSize-1;
				else r--;
			}

			// copy old digits
			for (sizeType j = 0; j < mSize; j++, i++)
				newData[i] = mDigits[j];

			// update members
			delete[] mDigits;
			mDigits = newData;
			mSize = newSize;
			mRepetendStart = 0;
			mIntegerStart += numNewDigits;
			
			returnVal = true;
		}
	}

	updateState();

	return returnVal;
}

// Equivalent to dividing the number by 10^shiftAmt
// Returns true if digits were added, false otherwise
bool bigNum::shiftRight(sizeType shiftAmt) {
	if (shiftAmt == 0) return false;

	// shift the radix point
	mIntegerStart += shiftAmt;

	// add additional zeros
	if (mIntegerStart >= mSize) {
		sizeType numZeros = mIntegerStart - mSize + 1;
		sizeType newSize = mSize + numZeros;
		digitType *newData = new digitType[newSize];

		// Copy the digits over, starting from the LSB
		sizeType i = 0;
		for (; i < mSize; i++)
			newData[i] = mDigits[i];

		// Pad the zeros beyond the MSB
		for (; i < newSize; i++)
			newData[i] = 0;

		delete[] mDigits;
		mDigits = newData;
		mSize = newSize;
	}

	updateState();
	
	return true;
}

bool bigNum::padMSB(sizeType numZeros) {	// every bigNum will have at least one integer digit; no change to mIntegerStart
	if (numZeros <= 0) return false;

	sizeType newSize = mSize + numZeros;
	digitType *newData = new digitType[newSize];
	sizeType i = 0;

	// Copy the digits over, starting from the LSB
	for (; i < mSize; i++)
		newData[i] = mDigits[i];

	// Pad the zeros beyond the MSB
	for (; i < newSize; i++)
		newData[i] = 0;

	delete[] mDigits;
	mDigits = newData;
	mSize = newSize;

	return true;
}

// Returns true if digits were added, false otherwise
bool bigNum::padLSB(sizeType numDigits, bool digitsAreFrac /* = false */) {
	if (numDigits <= 0) return false;						

	sizeType newSize = mSize + numDigits;
	digitType *newData = new digitType[newSize];
	sizeType i = 0;

	// add new digits
	if (mRepetendSize > 0) {	// Extend the repetend
		for (sizeType r = mRepetendStart + (mRepetendSize-1); i < numDigits; i++) {
			sizeType index = (numDigits-1) - i;
			newData[index] = mDigits[r];
			if (r == mRepetendStart) r += mRepetendSize-1;
			else r--;
		}
	} else						// Pad zeros
		for (; i < numDigits; i++)
			newData[i] = 0;

	// copy old digits
	for (sizeType j = 0; j < mSize; i++, j++)
		newData[i] = mDigits[j];

	// update members
	delete[] mDigits;
	mDigits = newData;
	mSize = newSize;
	if (mRepetendSize > 0) mRepetendStart += numDigits;
	if (mIntegerStart > 0 || digitsAreFrac) mIntegerStart += numDigits;

	updateState();

	return true;
}

/* Default constructor */
bigNum::bigNum(sizeType length /* = 50 */) {
	if (length < 1) length = 50;
	mSize = length;
	mNumDigits = 1;
	mIntegerStart = 0;
	mRepetendStart = 0;
	mRepetendSize = 0;
	mIsNegative = false;
	mIsInteger = true;
	mIsZero = true;
	mDigits = new digitType[length];
	for (sizeType d = 0; d < length; d++)
		mDigits[d] = 0;
}

bigNum::bigNum(std::string number) {
	if (number.length() == 0)
		throw std::length_error("Bignum cannot be constructed with no digits");

	// Check if the number is negative
	if (number[0] == '-') {
		mIsNegative = true;
		number.erase(0, 1);
	} else mIsNegative = false;

	// Check if the number contains a fractional part
	mIntegerStart = 0;
	mIsInteger = true;
	size_t i = 0;
	for (; i < number.length(); i++) {
		if (number[i] == '.' || number[i] == ',') {
			number.erase(i, 1);
			mIntegerStart = number.length() - i;
			if (i == 0) number.insert(number.begin(), '0');		// number was entered with no integer digits, add a zero in the one's place
			break;
		}
	}

	// Check if the number is an integer
	for (; i < number.length(); i++) {
		if (number[i] > '0' && number[i] <= '9') {
			mIsInteger = false;
			break;
		}
	}

	// Read in the digits
	if (number.length() == 0) throw std::length_error("Bignum cannot be constructed with no digits");
	mSize = number.length();
	mDigits = new digitType[mSize];
	for (sizeType d = 0; d < number.length(); d++) {
		if (number[d] < '0' || number[d] > '9')
			throw std::invalid_argument("Number contains an illegal character");
		mDigits[d] = number[(mSize - 1) - d] - '0';		// digits are stored little-endian
	}

	// Update state variables
	updateState();
	mRepetendStart = 0;
	mRepetendSize = 0;
}

/* Copy constructor */
bigNum::bigNum(const bigNum &rhs) {
	mSize = rhs.mSize;
	mNumDigits = rhs.mNumDigits;
	mIntegerStart = rhs.mIntegerStart;
	mRepetendStart = rhs.mRepetendStart;
	mRepetendSize = rhs.mRepetendSize;
	mIsNegative = rhs.mIsNegative;
	mIsInteger = rhs.mIsInteger;
	mIsZero = rhs.mIsZero;
	mDigits = new digitType[rhs.mSize];
	for (sizeType d = 0; d < rhs.mSize; d++)
		mDigits[d] = rhs.mDigits[d];
}

/* Copy assignment operator */
bigNum& bigNum::operator=(const bigNum& rhs) {
	if (this != &rhs) {				// check that this is not a self-assignment
		if (rhs.mSize != mSize) {	// check if a new size is needed
			delete[] mDigits;
			mSize = rhs.mSize;
			mDigits = new digitType[rhs.mSize];
		}
		mNumDigits = rhs.mNumDigits;
		mIntegerStart = rhs.mIntegerStart;
		mRepetendStart = rhs.mRepetendStart;
		mRepetendSize = rhs.mRepetendSize;
		mIsNegative = rhs.mIsNegative;
		mIsInteger = rhs.mIsInteger;
		mIsZero = rhs.mIsZero;
		for (sizeType d = 0; d < rhs.mSize; d++)
			mDigits[d] = rhs.mDigits[d];
	}
	return *this;
}

/* Destructor */
bigNum::~bigNum() {
	delete[] mDigits;
}

sizeType bigNum::numDigits() const {
	return mNumDigits;
}

sizeType bigNum::repetendPeriod() const {
	return mRepetendSize;
}

bool bigNum::isNegative() const {
	return mIsNegative;
}

bool bigNum::isInteger() const {
	return mIsInteger;
}

void bigNum::print(sizeType firstNDigits /* = 0 */) const {
	if (mSize == 0) {
		std::cout << "No digits found\n";
		return;
	}

	if (firstNDigits == 0) firstNDigits = mNumDigits;	// no number of digits was specified, print all
	else std::cout << "Displaying the first " << firstNDigits << " digits\n";
	if (mIsNegative) std::cout << "-";

	// Skip any leading zeros
	sizeType end = mSize - 1;
	while (end > mIntegerStart && mDigits[end] == 0) end--;

	sizeType index;
	for (sizeType i = 0; i < firstNDigits; i++) {
		index = end - i;
		if (mRepetendSize > 0 && index == (mRepetendStart + mRepetendSize-1)) std::cout << "(";
		std::cout << mDigits[index];
		if (mRepetendSize > 0 && index == mRepetendStart) {
			std::cout << ")";
			break;
		}
		if (index == mIntegerStart && i != (firstNDigits - 1)) std::cout << ".";

	}
	std::cout << "\n";
}

void bigNum::printAll() const {
	if (mSize == 0) {
		std::cout << "No digits found\n";
		return;
	}
	if (mIsNegative) std::cout << "-";
	sizeType index;
	for (sizeType i = 0; i < mSize; i++) {
		index = mSize - 1 - i;
		if (mRepetendSize > 0 && index == (mRepetendStart + mRepetendSize-1)) std::cout << "(";
		std::cout << mDigits[index];
		if (mRepetendSize > 0 && index == mRepetendStart) std::cout << ")";
		if (index == mIntegerStart && i != (mSize - 1)) std::cout << ".";
	}
	std::cout << "\nmSize: " << mSize;
	std::cout << "\nmNumDigits: " << mNumDigits;
	std::cout << "\nmIntegerStart: " << mIntegerStart;
	std::cout << "\nmRepetendStart: " << mRepetendStart;
	std::cout << "\nmRepetendSize: " << mRepetendSize;
	std::cout << "\nmIsNegative: " << mIsNegative;
	std::cout << "\nmIsInteger: " << mIsInteger;
	std::cout << "\nmIsZero: " << mIsZero;
	std::cout << "\n";
}

// Indexed from left to right, i.e. the most significant digit is index 0
digitType bigNum::operator[](sizeType d) const {
	if (d < 0 || d >= mSize)
		throw std::range_error("Index out of range");
	return mDigits[mSize-1-d];
}

// Indexed from left to right, i.e. the most significant digit is index 0
digitType& bigNum::operator[](sizeType d) {
	if (d < 0 || d >= mSize)
		throw std::range_error("Index out of range");
	return mDigits[mSize-1-d];
}

/* -------------------------------- */
/*      Comparison Operators        */
/* ---------------------------------*/

// update with repetend
bool bigNum::operator==(const bigNum &rhs) const {	// Test for equality ignores leading and trailing zeros
	bigNum lhs = *this;

	if (lhs.mIsZero && rhs.mIsZero) return true;
	if (lhs.mIsNegative != rhs.mIsNegative) return false;
	if (lhs.mIsInteger != rhs.mIsInteger) return false;
	if (lhs.mNumDigits != rhs.mNumDigits) return false;

	// Skip trailing zeros
	sizeType lhsStartIndex = 0, rhsStartIndex = 0;
	while (lhsStartIndex < lhs.mIntegerStart && lhs[lhsStartIndex] == 0) lhsStartIndex++;
	while (rhsStartIndex < rhs.mIntegerStart && rhs[rhsStartIndex] == 0) rhsStartIndex++;
	if ((lhs.mIntegerStart - lhsStartIndex) != (rhs.mIntegerStart - rhsStartIndex)) return false;	// make sure radix point is in the same place

	for (sizeType i = lhsStartIndex, j = rhsStartIndex; i < lhs.mNumDigits; i++, j++)
		if (lhs.mDigits[i] != rhs.mDigits[j]) return false;

	return true;
}

bool bigNum::operator!=(const bigNum &rhs) const {
	return !(*this == rhs);
}

// update with repetend
bool bigNum::operator>(const bigNum &rhs) const {
	bigNum lhs = *this;

	// Check if one number is + and one is -
	if (!lhs.isNegative() && rhs.isNegative()) return true;
	if (lhs.isNegative() && !rhs.isNegative()) return false;

	// Check if one number has more integer digits
	sizeType lhsMSIntDigitPos = lhs.mSize - 1;
	sizeType rhsMSIntDigitPos = rhs.mSize - 1;
	while (lhsMSIntDigitPos > lhs.mIntegerStart && lhs[lhsMSIntDigitPos] == 0) lhsMSIntDigitPos--;
	while (rhsMSIntDigitPos > rhs.mIntegerStart && rhs[rhsMSIntDigitPos] == 0) rhsMSIntDigitPos--;
	if ((lhsMSIntDigitPos - lhs.mIntegerStart) > (rhsMSIntDigitPos - rhs.mIntegerStart)) return true;
	if ((rhsMSIntDigitPos - rhs.mIntegerStart) > (lhsMSIntDigitPos - lhs.mIntegerStart)) return false;

	// Compare the digits
	sizeType l = lhsMSIntDigitPos, r = rhsMSIntDigitPos;
	sizeType i = 0;
	for (; i < lhs.mNumDigits && i < rhs.mNumDigits; i++) {
		if (lhs[l-i] > rhs[r-i]) return true;
		if (rhs[r-i] > lhs[l-i]) return false;
	}
	
	// If the lhs has more fraction digits, check if any are non-zero
	while (i < lhs.mNumDigits) {
		if (lhs[l - i] > 0) return true;
		i++;
	}

	// lhs <= rhs
	return false;
}

bool bigNum::operator<(const bigNum &rhs) const {
	bigNum lhs = *this;

	// Check if the numbers are equal before using operator>
	if (lhs == rhs) return false;

	return !(lhs > rhs);
}

bool bigNum::operator>=(const bigNum &rhs) const {
	bigNum lhs = *this;

	// Check if the numbers are equal before using operator>
	if (lhs == rhs) return true;

	return (lhs > rhs);
}

bool bigNum::operator<=(const bigNum &rhs) const {
	bigNum lhs = *this;

	// Check if the numbers are equal before using operator>
	if (lhs == rhs) return true;

	return !(lhs > rhs);
}


/* -------------------------------- */
/*             Addition             */
/* ---------------------------------*/

// !-- Fix for an operand with max number of digits
bigNum bigNum::absValAdd(bigNum &a, bigNum &b) const {

	// Make both integer parts the same length
	sizeType aIntDigits = a.mSize - a.mIntegerStart;
	sizeType bIntDigits = b.mSize - b.mIntegerStart;
	if (aIntDigits > bIntDigits) b.padMSB(aIntDigits - bIntDigits);
	else if (aIntDigits < bIntDigits) a.padMSB(bIntDigits - aIntDigits);

	// Make both fraction parts the same length
	sizeType aFracDigits = a.mIntegerStart;
	sizeType bFracDigits = b.mIntegerStart;
	if (aFracDigits > bFracDigits) b.padLSB(aFracDigits - bFracDigits, true);
	else if (aFracDigits < bFracDigits) a.padLSB(bFracDigits - aFracDigits, true);

	// The sum may contain at most 1 additional digit
	sizeType resultSize = a.mSize + 1;
	bigNum result(resultSize);
	
	// Add the digits, starting from the LSB
	digitType carry = 0;
	sizeType d;
	for (d = 0; d < a.mSize; d++) {
		result.mDigits[d] = a.mDigits[d] + b.mDigits[d] + carry;
		if (result.mDigits[d] > 9) {
			carry = 1;
			result.mDigits[d] -= 10;
		} else carry = 0;
	}
	if (carry == 1) result.mDigits[d] = 1;		// the sum contains an extra digit

	// Update the decimal point position
	result.mIntegerStart = a.mIntegerStart;
	result.updateState();

	return result;
}

bigNum bigNum::operator+(const bigNum &other) const {
	bigNum a = *this;
	bigNum b = other;
	bigNum result;
	if (a.isNegative()) {
		if (b.isNegative()) {				// -a -b
			result = absValAdd(a, b);
			result.mIsNegative = true;
		} else {							// -a +b
			result = absValSubtract(b, a);
		}
	} else {
		if (b.isNegative()) {				// +a -b
			result = absValSubtract(a, b);
		} else {							// +a +b
			result = absValAdd(a, b);
		}
	}

	return result;
}

/* ------------------------------------------------------- */
/*            Subtraction (Method of complements)          */
/* --------------------------------------------------------*/

bigNum bigNum::absValSubtract(bigNum &lhs, bigNum &rhs) const {

	// Minuend - subtrahend = difference

	// Make sure the minuend is > subtrahend
	bigNum &min = (lhs > rhs) ? lhs : rhs;
	bigNum &sub = (lhs > rhs) ? rhs : lhs;
	bool differenceIsNeg = (rhs > lhs) ? true : false;

	// Make both integer parts the same length
	sizeType minIntDigits = min.mSize - min.mIntegerStart;
	sizeType subIntDigits = sub.mSize - sub.mIntegerStart;
	if (minIntDigits > subIntDigits) sub.padMSB(minIntDigits - subIntDigits);
	else if (minIntDigits < subIntDigits) min.padMSB(subIntDigits - minIntDigits);

	// Make both fraction parts the same length
	sizeType minFracDigits = min.mIntegerStart;
	sizeType subFracDigits = sub.mIntegerStart;
	if (minFracDigits > subFracDigits) sub.padLSB(minFracDigits - subFracDigits, true);
	else if (minFracDigits < subFracDigits) min.padLSB(subFracDigits - minFracDigits, true);
	
	// Calculate the 9's complement of the minuend
	bigNum minComplement(min.mSize);
	for (sizeType i = 0; i < minComplement.mSize; i++)
		minComplement.mDigits[i] = 9 - min.mDigits[i];

	// Add the 9's complement of the minuend to the subtrahend
	bigNum sum(min.mSize);
	digitType carry = 0;
	for (sizeType d = 0; d < sum.mSize; d++) {
		sum.mDigits[d] = minComplement.mDigits[d] + sub.mDigits[d] + carry;
		if (sum.mDigits[d] > 9) {
			carry = 1;
			sum.mDigits[d] -= 10;
		} else carry = 0;
	}

	// Calculate the 9's complement of the sum
	bigNum dif(min.mSize);
	for (sizeType i = 0; i < dif.mSize; i++)
		dif.mDigits[i] = 9 - sum.mDigits[i];

	// Update state
	dif.mIntegerStart = min.mIntegerStart;
	dif.mIsNegative = differenceIsNeg;
	dif.updateState();
	
	return dif;
}

bigNum bigNum::operator-(const bigNum &other) const {
	bigNum lhs = *this;
	bigNum rhs = other;
	bigNum result;
	if (lhs.isNegative()) {
		if (rhs.isNegative()) {				// (-lhs) - (-rhs)
			result = absValSubtract(rhs, lhs);
		} else {							// (-lhs) - (+rhs)
			result = absValAdd(rhs, lhs);
			result.mIsNegative = true;
		}
	} else {
		if (rhs.isNegative()) {				// (+lhs) - (-rhs)
			result = absValAdd(lhs, rhs);
		} else {							// (+lhs) - (+rhs)
			result = absValSubtract(lhs, rhs);
		}
	}

	return result;
}

/* --------------------------------------------------- */
/*           Multiplication (Karatsuba Method)         */
/* ----------------------------------------------------*/

uint64_t toUInt64(const bigNum &n) { // ignores mIsNegative and mIntegerStart

	uint64_t UINT64_T_MAX = std::numeric_limits<uint64_t>::max();
	bigNum max(std::to_string(UINT64_T_MAX));
	if (n > max) return 0;

	sizeType end = n.mSize - 1;
	while (end > n.mIntegerStart && n.mDigits[end] == 0) end--;

	std::stringstream ss;
	for (sizeType d = 0; d < n.mNumDigits; d++)
		ss << n.mDigits[end - d];

	uint64_t result;
	ss >> result;

	return result;
}

bool bigNum::splitAt(sizeType pos, bigNum &low, bigNum &high) const {
	if (low.mSize < pos || high.mSize < (mSize - pos)) return false;
	sizeType i;
	for (i = 0; i < pos; i++)
		low.mDigits[i] = mDigits[i];
	for (sizeType j = 0; i < mSize; i++, j++)
		high.mDigits[j] = mDigits[i];
	low.updateState();
	high.updateState();
	return true;
}

bool bigNum::setKaratsubaDecimalPoint(const bigNum &lhs, const bigNum &rhs) {
	
	// Count the fractional digits, excluding trailing zeros
	sizeType startL = 0, startR = 0;
	while (lhs.mDigits[startL] == 0 && startL < lhs.mIntegerStart) startL++;
	while (rhs.mDigits[startR] == 0 && startR < rhs.mIntegerStart) startR++;
	sizeType numFracDigitsLHS = lhs.mIntegerStart - startL;
	sizeType numFracDigitsRHS = rhs.mIntegerStart - startR;

	// Add the number of fractional digits of both operands
	mIntegerStart = numFracDigitsLHS + numFracDigitsRHS;

	// If either operand's most significant digits begin with 0, the result will be missing leading zeros
	if (mIntegerStart >= mSize)
		padMSB(mIntegerStart - mSize + 1);

	return true;
}

bigNum bigNum::integerKaratsuba(bigNum a, bigNum b) const {

	// For operands under 10 digits, use 64-bit integer multiplication
	if (a.mNumDigits < 10 && b.mNumDigits < 10) {
		uint64_t i = toUInt64(a);
		uint64_t j = toUInt64(b);
		uint64_t p = i*j;
		return bigNum(std::to_string(p));
	}

	// Set both bigNums to the same length
	if (a.mSize > b.mSize) b.padMSB(a.mSize - b.mSize);
	else if (a.mSize < b.mSize) a.padMSB(b.mSize - a.mSize);

	// Choose m
	sizeType m = a.mSize / 2 + (a.mSize % 2 != 0);	// m = ceiling(mSize/2)

	// Split both bigNums into a low and high part separated at position m
	bigNum aLow(m), aHigh(m);
	bigNum bLow(m), bHigh(m);
	a.splitAt(m, aLow, aHigh);
	b.splitAt(m, bLow, bHigh);

	/* Calculate Karatsuba equations, recursively call this function to perform the multiplications
		z0 = (aLow)*(bLow)
		z2 = (aHigh)*(bHigh)
		z1 = (aLow + aHigh)*(bLow + bHigh) - z0 - z2 */
	bigNum z0 = integerKaratsuba(aLow, bLow);
	bigNum z2 = integerKaratsuba(aHigh, bHigh);
	bigNum z1 = absValSubtract(absValSubtract((integerKaratsuba(absValAdd(aLow, aHigh), absValAdd(bLow, bHigh))), z0), z2);

	/* Compute the result using these intermediate values
		result = ((10^(2*m)) * z2) + ((10^m) * z1) + z0 */
	z2.shiftLeft(2 * m);
	z1.shiftLeft(m);
	bigNum result = absValAdd(absValAdd(z0, z2), z1);

	return result;
}

// Does not sanitize result (leaves leading/trailing zeros)
bigNum bigNum::absValMultiply(bigNum a, bigNum b) const {

	// Remove extra leading and trailing zeros from the operands
	a.removeLeadingZeros();
	a.removeTrailingZeros();
	b.removeLeadingZeros();
	b.removeTrailingZeros();

	// Perform karatsuba multiplication 
	bigNum result = integerKaratsuba(a, b);
	result.setKaratsubaDecimalPoint(a, b);
	result.updateState();

	return result;
}

// Sanitizes result (removes leading/trailing zeros)
bigNum bigNum::operator*(const bigNum &other) const {

	bigNum a = *this;
	bigNum b = other;

	// Check if either operand is zero
	if (a.mIsZero || b.mIsZero)
		return bigNum("0");

	bigNum result = absValMultiply(a, b);

	// Remove leading and trailing zeros
	result.removeLeadingZeros();
	result.removeTrailingZeros();

	// Check negative operands
	if (a.mIsNegative != b.mIsNegative)
		result.mIsNegative = true;

	return result;
}

/* ------------------------------------------------------- */
/*             Division (Newton-Raphson Method)            */
/* --------------------------------------------------------*/

bigNum bigNum::round(const bigNum &n, sizeType precision) const {
	if (precision == 0)
		throw std::length_error("Bignum cannot be constructed with no digits");

	if (precision >= n.mNumDigits) {
		bigNum result = n;		// make a copy of n
		return result;
	}

	bigNum result(n.mNumDigits);

	// Set all digits outside the result's precision to 0
	sizeType i = 0;
	for (; i < n.mNumDigits - precision; i++)
		result.mDigits[i] = 0;

	// Copy over each digit, rounding when necessary
	digitType carry = n[i-1] >= 5 ? 1 : 0;
	for (; i < n.mNumDigits; i++) {
		result.mDigits[i] = n.mDigits[i] + carry;
		if (result.mDigits[i] > 9) {
			carry = 1;
			result.mDigits[i] -= 10;
		} else carry = 0;
	}

	// If the result's MSB is rounded above 9 (e.g. rounding 9.99), the result requires an extra digit
	if (carry == 1) {
		bigNum result1(n.mNumDigits + 1);
		sizeType i;
		for (i = 0; i < result.mSize; i++)
			result1.mDigits[i] = result.mDigits[i];
		result1.mDigits[i] = carry;
		result = result1;
	}

	// Check if the rounded number is 0 and update mIsNegative and mIntegerStart
	result.mIntegerStart = n.mIntegerStart;
	result.mIsNegative = n.mIsNegative;
	result.updateState();
	
	return result;
}

//bigNum bigNum::newtonRaphsonDivide(const bigNum &lhs, const bigNum &rhs) const {
//
//	// Newton-Raphson Division
//	// Q = N/D
//	// 1. Calculate an estimate X0 for the reciprocal (1/D) of the divisor
//	// 2. Compute successively more accurate estimates X1, X2, ... of the reciprocal
//	// 3. Compute the quotient by multiplying the numerator by the reciprocal Xn
//
//	bigNum N = lhs;
//	bigNum D = rhs;
//	digitType precision = 50;							// CONSIDER CHANGING TO SOME OTHER VALUE
//
//														// Scale denominator to be in [0,1)
//	int e = D.mNumDigits - D.mIntegerStart;				// FIX THIS FOR NUMBERS < 0
//	bigNum Dnorm = D;
//	Dnorm.mIntegerStart += e;						// FIX: DECIMALPOINTPOS MAY BE UPDATED BEYOND MSIZE
//
//														// Scale numerator by the same amount
//	bigNum Nnorm = N;
//	Nnorm.mIntegerStart += e;						// FIX: DECIMALPOINTPOS MAY BE UPDATED BEYOND MSIZE
//
//														// Calculate an initial guess X0 for 1/Dnorm		// FIX THIS FOR ANY LENGTH DNORM
//														// X0 = A - B(Dnorm)								// FIX CONSTANTS MUST BE ROUNDED NOT TRUNCATED
//	std::string a = divConstantA.substr(0, Dnorm.mNumDigits + 2);	// Calculate the constants to the same
//	std::string b = divConstantB.substr(0, Dnorm.mNumDigits + 2);	// precision as Dnorm
//	bigNum A = round(bigNum(a), Dnorm.mNumDigits);
//	bigNum B = round(bigNum(b), Dnorm.mNumDigits);
//	bigNum X0 = A - (B * Dnorm);
//
//	// Refine the initial guess by iterating through:
//	// Xn = Xn-1 + Xn-1 * (1 - Dnorm * Xn-1)^
//	bigNum Xn = X0, Xnminus1 = X0;
//	bigNum one("1");
//	bigNum two("2");
//	for (int i = 0; i < 5; i++) {						// FIX THIS WITH THE CORRECT # OF ITERATIONS
//		Xnminus1 = Xn;
//		Xn = Xnminus1 + (Xnminus1 * (one - (Dnorm * Xnminus1)));	// FIX THIS BY LIMITING PRECISION OF EACH   <--------- this next
//		Xn = absValAdd(Xnminus1, integerKaratsuba(Xnminus1, absValSubtract(one, integerKaratsuba(Dnorm, Xnminus1))));
//		//Xn = Xnminus1 * (two - (Dnorm * Xnminus1));
//	}															// MULTIPLICATION TO FINAL (?) PRECISION
//
//																// Calculate the quotient by multiplying Nnorm by the final iteration
//	bigNum result = Nnorm * Xn;
//
//	return result;
//}


/* ------------------------------------------------------- */
/*             Division (Long Division)                    */
/* --------------------------------------------------------*/

bigNum bigNum::extractBigNum(sizeType lsb, sizeType msb) const {
	
	if (msb < lsb || msb >= mSize) throw std::invalid_argument("Bad indexes in extractBigNum");

	sizeType length = msb - lsb + 1;
	std::stringstream ss;
	for (sizeType i = 0; i < length; i++)
		ss << mDigits[msb - i];		// construct string starting from the msb

	return bigNum(ss.str());
}

bigNum bigNum::multiplyByDigit(digitType d) const {
	bigNum result(mSize);

	digitType carry = 0;
	for (sizeType i = 0; i < mSize; i++) {
		result.mDigits[i] = mDigits[i] * d + carry;
		if (result.mDigits[i] > 9) {
			carry = result.mDigits[i] / 10;
			result.mDigits[i] %= 10;
		} else carry = 0;
	}
	if (carry != 0) {	// result of multiplication requires 1 extra digit
		digitType *newData = new digitType[result.mSize + 1];
		for (sizeType i = 0; i < result.mSize; i++)
			newData[i] = result.mDigits[i];
		newData[result.mSize] = carry;
		delete[] result.mDigits;
		result.mDigits = newData;
		result.mSize++;
	}

	result.updateState();

	return result;
}

// Returns a quotient and remainder
bigNum bigNum::divideByDigit(digitType d, bigNum& remainder) const {

	if (d == 0) throw std::invalid_argument("Division by zero");

	bigNum quotient(mSize);
	digitType r = 0;

	for (sizeType i = 0; i < mSize; i++) {
		quotient.mDigits[(mSize-1) - i] = (10*r + mDigits[(mSize-1) - i]) / d;
		r = (10*r + mDigits[(mSize-1) - i]) % d;
	}

	remainder = bigNum(std::to_string(r));

	quotient.updateState();
	remainder.updateState();

	return quotient;
}

// Returns true if the division results in a terminating or repeating decimal, false otherwise
bool bigNum::precisionDivideByDigit(digitType d, const bigNum& remainder, bigNum& quotient) const {

	// Divide remainder by d up to a precision of 9 and store the result in quotient

	if (d == 0) throw std::invalid_argument("Division by zero");
	if (remainder.mNumDigits > 1 || remainder.mSize < 1 || remainder.mIntegerStart != 0)	// remainder must be 1 digit
		throw std::invalid_argument("Invalid remainder in precisionDivideByDigit");
	
	bigNum u = *this;
	digitType r = remainder[0];
	std::unordered_map<digitType, sizeType> remainders;	// keep track of remainders to detect a repetend
	remainders.emplace(d, 0);
	bool returnVal = false;

	// Make space in the quotient for up to 9 digits of precision
	sizeType precision = 9;
	quotient.padLSB(precision, true);

	// Divide r by d until the resulting quotient terminates or repeats
	for (sizeType i = 0; i < precision; i++) {
		quotient.mDigits[precision - 1 - i] = (10*r) / d;
		r = (10*r) % d;

		// If the remainder is 0, the result of the division terminates
		if (r == 0) {
			returnVal = true;
			break;
		}

		// Check if this remainder has been seen before --> indicates the existence of a repetend
		auto it = remainders.find(r);
		if (it != remainders.end()) {
			quotient.mRepetendSize = (i - it->second) + 1;
			quotient.mRepetendStart = precision - 1 - i;
			returnVal = true;
			break;
		} else remainders.emplace(r, i + 1);
	}

	quotient.removeTrailingZeros();
	quotient.updateState();

	return returnVal;
}

// Returns a quotient and remainder
bigNum bigNum::integerLongDivision(const bigNum &dividend, const bigNum &divisor, bigNum& remainder) const {

	bigNum u = dividend;
	bigNum v = divisor;

	// Remove extra leading and trailing zeros
	u.removeLeadingZeros();
	u.removeTrailingZeros();
	v.removeLeadingZeros();
	v.removeTrailingZeros();

	// Requires dividend.mSize >= divisor.mSize
	if (u.mSize < v.mSize) throw std::invalid_argument("Invalid operator size in precisionLongDivision");
	if (v.mNumDigits == 1) return u.divideByDigit(v.mDigits[0], remainder);

	sizeType n = v.mSize + 1;				// size of partial dividend
	sizeType m = u.mSize - v.mSize + 1;		// size of quotient
	bigNum quotient(m);
	
	// Normalize the dividend and divisor so that the first digit of the divisor is >= 5
	digitType d = 10 / (v[0] + 1);
	sizeType uSize = u.mSize;
	u = u.multiplyByDigit(d);
	v = v.multiplyByDigit(d);
	if (u.mSize == uSize) u.padMSB(1);		// Ensure u has one extra leading digit

	// For each iteration, divide the partial dividend by v in-place to obtain one digit (q) of the final quotient and a partial quotient
	for (sizeType i = 0; i < m; i++) {

		// Partial dividend consists of the digits u[i] to u[i+n-1]
		
		// Make a guess for q by dividing the first two digits of the partial dividend u[i]u[i+1] by v[0] and adjusting if the guess is too high
		digitType q = (u[i] == v[0]) ? 9 : (u[i] * 10 + u[i+1]) / v[0];	
		while ((v[1]*q) > (10*(u[i]*10 + u[i+1] - q*v[0]) + u[i+2])) q--;

		// Compute the partial quotient by subtracting q*v from the partial dividend in-place
		bigNum product = v.multiplyByDigit(q);
		if (product.mSize != (n)) product.padMSB((n) - product.mSize);
		sizeType l = (u.mSize-1) - (n-1) - i;	// Index of partial dividend's least significant digit

		// (subtraction done using method of complements)
		digitType carry = 0;
		for (sizeType j = 0; j < n; j++) {		// Complement the minuend and add the subtrahend
			u.mDigits[l + j] = 9 - u.mDigits[l + j] + product.mDigits[j] + carry;
			if (u.mDigits[l + j] > 9) {
				carry = 1;
				u.mDigits[l + j] -= 10;
			} else carry = 0;
		}
		if (carry == 0) {						// Complementing gives the partial quotient
			for (sizeType j = 0; j < n; j++)
				u.mDigits[l + j] = 9 - u.mDigits[l + j];
		} else if (carry == 1) {				// Result is negative - q was 1 too large
			
			// "End-around carry" - add the carry to the LSB to get the difference (a negative value)
			for (sizeType j = 0; j < n; j++) {
				u.mDigits[l + j] += carry;
				if (u.mDigits[l + j] > 9) u.mDigits[l + j] -= 10;
				else break;
			}
			
			// Add back v to get the correct partial quotient (done by subtracting the partial quotient from v since the partial quotient was negative)
			carry = 0;
			for (sizeType j = 0; j < (n - 1); j++) {
				u.mDigits[l + j] += 9 - v.mDigits[j] + carry;
				if (u.mDigits[l + j] > 9) {
					carry = 1;
					u.mDigits[l + j] -= 10;
				} else carry = 0;
			}
			u.mDigits[n-1] = 9;					// Carry will always be 0 for the last digit
			for (sizeType j = 0; j < n; j++)	// Complementing gives the partial quotient
				u.mDigits[l + j] = 9 - u.mDigits[l + j];

			q--;
		}

		// Set this digit of the final quotient
		quotient[i] = q;
	}

	// Denormalize the dividend to get the remainder
	remainder = u.divideByDigit(d, v);
	remainder.removeLeadingZeros();
	remainder.updateState();

	// Update state
	quotient.updateState();

	return quotient;
}

// Returns true if the division results in a terminating or repeating decimal, false otherwise
bool bigNum::precisionLongDivision(const bigNum& remainder, const bigNum& divisor, bigNum& quotient, sizeType precision) const {

	if (precision <= 0) throw std::invalid_argument("Invalid precision in precisionLongDivision");
	if (remainder.mSize > (divisor.mSize + 1)) throw std::invalid_argument("Invalid operator size in precisionLongDivision");
	if (remainder.mSize == 1 && remainder.mDigits[0] == 0) return true;

	bigNum u = remainder;
	bigNum v = divisor;
	bool returnVal = false;

	// Remove extra leading and trailing zeros
	u.removeLeadingZeros();
	u.removeTrailingZeros();
	v.removeLeadingZeros();
	v.removeTrailingZeros();

	if (v.mNumDigits == 1) return u.precisionDivideByDigit(v.mDigits[0], remainder, quotient);

	// Make space in the quotient for # of digits up to the specified precision
	quotient.padLSB(precision, true);

	// Multiply the initial remainder by 10
	u.shiftLeft(1);

	// Normalize the dividend and divisor so that the first digit of the divisor is >= 5
	digitType d = 10 / (v[0] + 1);
	u = u.multiplyByDigit(d);
	v = v.multiplyByDigit(d);
	if (u.mSize < (v.mSize + 1)) u.padMSB(v.mSize + 1 - u.mSize);

	sizeType n = u.mSize;

	std::unordered_map<bigNum, sizeType> remainders;	// keep track of remainders to detect a repetend
	remainders.emplace(u, 0);

	// For each iteration, multiply the remainder by 10 and divide it by v in-place to obtain one digit (q) of the quotient and a new remainder
	for (sizeType i = 0; i < precision; i++) {

		// Make a guess for q by dividing the first two digits of the remainder u[0]u[1] by v[0] and adjusting if the guess is too high
		digitType q = (u[0] == v[0]) ? 9 : (u[0] * 10 + u[1]) / v[0];
		while (v[1]*q > (10*(u[0]*10 + u[1] - q*v[0]) + u[2])) q--;

		// Compute the next remainder by subtracting q*v from the remainder in-place
		bigNum product = v.multiplyByDigit(q);
		if (product.mSize < n) product.padMSB(n - product.mSize);

		// (subtraction done using method of complements)
		digitType carry = 0;
		for (sizeType j = 0; j < n; j++) {				// Complement the minuend and add the subtrahend
			u.mDigits[j] = 9 - u.mDigits[j] + product.mDigits[j] + carry;
			if (u.mDigits[j] > 9) {
				carry = 1;
				u.mDigits[j] -= 10;
			} else carry = 0;
		}
		if (carry == 0) {								// Complementing gives the next remainder
			for (sizeType j = 0; j < (n - 1); j++)
				u.mDigits[n-1-j] = 9 - u.mDigits[n-2-j];// Shift to get remainder*10 
			u.mDigits[0] = 0;
		} else if (carry == 1) {						// Result is negative - q was 1 too large

			// "End-around carry" - add the carry to the LSB to get the difference (a negative value)
			for (sizeType j = 0; j < n; j++) {
				u.mDigits[j] += carry;
				if (u.mDigits[j] > 9) u.mDigits[j] -= 10;
				else break;
			}

			// Add back v to get the correct remainder (done by subtracting the remainder from v since the remainder was negative)
			carry = 0;
			for (sizeType j = 0; j < (n - 1); j++) {	// Complement the minuend and add the subtrahend
				u.mDigits[j] += 9 - v.mDigits[j] + carry;
				if (u.mDigits[j] > 9) {
					carry = 1;
					u.mDigits[j] -= 10;
				} else carry = 0;
			}
			u.mDigits[n-1] = 9;							// Carry will always be 0 for the last digit
			for (sizeType j = 0; j < (n - 1); j++)		// Complementing gives the next remainder
				u.mDigits[n-1-j] = 9 - u.mDigits[n-2-j];// Shift to get remainder*10
			u.mDigits[0] = 0;

			q--;
		}

		// Set this digit of the quotient
		quotient.mDigits[precision-1-i] = q;

		// If the remainder is 0, the result of the division terminates
		bool isZero = true;
		for (sizeType j = 0; j < n; j++)
			if (u.mDigits[j] != 0) {
				isZero = false;
				break;
			}
		if (isZero) {
			returnVal = true;
			break;
		}

		// Check if this remainder has been seen before --> indicates the existence of a repetend
		auto it = remainders.find(u);
		if (it != remainders.end()) {
			quotient.mRepetendSize = (i - it->second) + 1;
			quotient.mRepetendStart = precision - 1 - i;
			returnVal = true;
			break;
		} else remainders.emplace(u, i + 1);

	}

	quotient.removeTrailingZeros();
	quotient.updateState();

	return returnVal;
}

bigNum bigNum::operator/(const bigNum& divisor) const {
	
	bigNum u = *this;
	bigNum v = divisor;
	u.updateState();
	v.updateState();

	// Check if either operand is zero
	if (u.mIsZero) return bigNum("0");
	if (v.mIsZero) throw std::invalid_argument("Division by zero");

	// Convert both numbers to integers
	if (u.mIntegerStart > 0 || v.mIntegerStart > 0) {
		sizeType shiftAmount = (u.mIntegerStart > v.mIntegerStart) ? u.mIntegerStart : v.mIntegerStart;
		u.shiftLeft(shiftAmount);
		v.shiftLeft(shiftAmount);
	}

	// Make u.numDigits >= v.numDigits
	sizeType quotientScalingFactor = 0;
	if (u.mNumDigits < v.mNumDigits) {
		quotientScalingFactor = v.mNumDigits - u.mNumDigits;
		u.shiftLeft(quotientScalingFactor);
	}

	bigNum r;
	bigNum quotient = integerLongDivision(u, v, r);				// divide all given digits
	precisionLongDivision(r, v, quotient, defaultPrecision);	// divide extra digits up to defaultPrecision
	if (quotientScalingFactor) quotient.shiftRight(quotientScalingFactor);

	// Remove leading and trailing zeros
	quotient.removeLeadingZeros();
	quotient.removeTrailingZeros();

	// Check negative operands
	if (u.mIsNegative != v.mIsNegative)
		quotient.mIsNegative = true;

	return quotient;
}

// Returns true if the division results in a terminating or repeating decimal, false otherwise, does not round
bool divideWithPrecision(const bigNum& dividend, const bigNum& divisor, bigNum& quotient, sizeType p) {

	bigNum u = dividend;
	bigNum v = divisor;
	u.updateState();
	v.updateState();

	// Check if either operand is zero
	if (u.mIsZero) {
		quotient = bigNum("0");
		return true;
	}
	if (v.mIsZero) throw std::invalid_argument("Division by zero");

	// Convert both numbers to integers
	if (u.mIntegerStart > 0 || v.mIntegerStart > 0) {
		sizeType shiftAmount = (u.mIntegerStart > v.mIntegerStart) ? u.mIntegerStart : v.mIntegerStart;
		u.shiftLeft(shiftAmount);
		v.shiftLeft(shiftAmount);
	}

	// Make u.numDigits >= v.numDigits
	sizeType quotientScalingFactor = 0;
	if (u.mNumDigits < v.mNumDigits) {
		quotientScalingFactor = v.mNumDigits - u.mNumDigits;
		u.shiftLeft(quotientScalingFactor);
	}

	bigNum r;
	quotient = u.integerLongDivision(u, v, r);						// divide all given digits
	bool returnVal = u.precisionLongDivision(r, v, quotient, p);	// divide p extra digits
	if (quotientScalingFactor) quotient.shiftRight(quotientScalingFactor);

	// Remove leading and trailing zeros
	quotient.removeLeadingZeros();
	quotient.removeTrailingZeros();

	// Check negative operands
	if (u.mIsNegative != v.mIsNegative)
		quotient.mIsNegative = true;

	return returnVal;
}

bigNum bigNum::operator%(const bigNum& divisor) const {

	bigNum u = *this;
	bigNum v = divisor;

	if (v.mIsZero)
		throw std::invalid_argument("Division by zero");
	u.setPositive();
	v.setPositive();
	if (u < v) return u;

	// Convert both numbers to integers
	sizeType remainderScalingFactor = 0;
	if (u.mIntegerStart > 0 || v.mIntegerStart > 0) {
		sizeType shiftAmount = (u.mIntegerStart > v.mIntegerStart) ? u.mIntegerStart : v.mIntegerStart;
		u.shiftLeft(shiftAmount);
		v.shiftLeft(shiftAmount);
		remainderScalingFactor = shiftAmount;
	}

	bigNum remainder;
	integerLongDivision(u, v, remainder);

	if (remainderScalingFactor) remainder.shiftRight(remainderScalingFactor);

	remainder.removeLeadingZeros();
	remainder.removeTrailingZeros();
	remainder.mIsNegative = this->mIsNegative;	// sign of modulo matches sign of dividend

	return remainder;
}

/* -------------------------------- */
/*          Exponentiation          */
/* ---------------------------------*/

bigNum bigNum::operator^(uint32_t power) const {
	bigNum result("1");
	for (uint32_t i = 0; i < power; i++)
		result = *this * result;
	result.removeLeadingZeros();		// need this?
	return result;
}








/* -------------------------------- */
/*             Factorial		    */
/* ---------------------------------*/

bigNum factorial(const bigNum &n) {
	
	// Check the number is a non-negative integer
	if (!n.isInteger() || n.isNegative()) return bigNum("0");

	bigNum result = n;
	bigNum one("1");
	bigNum i = n - one;
	while (i != one) {
		result = result * i;
		i = i - one;
	}
	return result;
}









// Old subtraction algo using method of complements (method 2 on wiki)

//bigNum bigNum::absValSubtract(bigNum &lhs, bigNum &rhs) const {
//	
//	// Minuend - subtrahend = difference
//
//	// Make both integer parts the same length
//	sizeType lhsIntDigits = lhs.mSize - lhs.mIntegerStart;
//	sizeType rhsIntDigits = rhs.mSize - rhs.mIntegerStart;
//	if (lhsIntDigits > rhsIntDigits) rhs.padMSB(lhsIntDigits - rhsIntDigits);
//	else if (lhsIntDigits < rhsIntDigits) lhs.padMSB(rhsIntDigits - lhsIntDigits);
//
//	// Make both fraction parts the same length
//	sizeType lhsFracDigits = lhs.mIntegerStart;
//	sizeType rhsFracDigits = rhs.mIntegerStart;
//	if (lhsFracDigits > rhsFracDigits) rhs.padZerosLSB(lhsFracDigits - rhsFracDigits, true);
//	else if (lhsFracDigits < rhsFracDigits) lhs.padZerosLSB(rhsFracDigits - lhsFracDigits, true);
//
//	// Calculate the 9's complement of the subtrahend
//	bigNum ninesComplement(rhs.mSize);
//	for (sizeType i = 0; i < rhs.mSize; i++)
//		ninesComplement[i] = 9 - rhs.mDigits[i];
//	ninesComplement.mIntegerStart = rhs.mIntegerStart;
//	ninesComplement.updateState();
//
//	// Add the 9's complement of the subtrahend and the minuend
//	bigNum sum = absValAdd(lhs, ninesComplement);
//
//	bigNum result;
//
//	// If the sum contains an extra digit, "carry" it to the LSB by adding 1 and dropping the MSB
//	if (sum.mNumDigits > lhs.mNumDigits && sum.mNumDigits > rhs.mNumDigits) {	// result is positive
//		// "end-around carry"
//
//		result = bigNum(lhs.mSize);		// set the result's size to the size of an operand, dropping the MSB of the sum
//		digitType carry = 1;			// begin by adding 1 to the LSB
//
//		// Add the digits, starting from the LSB
//		for (sizeType d = 0; d < result.mSize; d++) {
//			result[d] = sum.mDigits[d] + result.mDigits[d] + carry;
//			if (result[d] > 9) {
//				carry = 1;
//				result[d] -= 10;
//			} else {
//				carry = 0;
//			}
//		}
//	}
//	// Else the result is negative, find the 9's complement
//	else {					// result is negative
//		bigNum sumComplement(sum.mSize);
//		for (sizeType i = 0; i < sum.mSize; i++)
//			sumComplement[i] = 9 - sum.mDigits[i];
//		result = sumComplement;
//		result.mIsNegative = true;
//	}
//	result.mIntegerStart = sum.mIntegerStart;
//	result.updateState();
//	return result;
//}



// Add'tl check in setKaratsubaDecimalPoint. Should not be needed now that every bigNum has an integer part?

// If either operand had no integer part, the result will be missing leading zeros
//if (result->mIntegerStart > result->mSize) {
//	sizeType oldSize = result->mSize;
//	sizeType numZeros = result->mIntegerStart - oldSize;
//	sizeType newSize = oldSize + numZeros;
//	digitType *newData = new digitType[newSize];
//	sizeType i;
//
//	// Copy the digits over, starting from the LSB
//	for (i = 0; i < oldSize; i++)
//		newData[i] = mDigits[i];
//
//	// Pad the missing zeros
//	for (; i < newSize; i++)
//		newData[i] = 0;
//
//	// Update mDigits and mSize
//	delete[] result->mDigits;
//	result->mDigits = newData;
//	result->mSize = newSize;
//}