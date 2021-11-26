#include "stdafx.h"
#include "bigNumOLD.h"
#include <sstream>
#include <string>
#include <limits>
#include <unordered_map>	// to store remainders during division to check for repetends
#include <utility>			// std::pair
#include <type_traits>

namespace std {
	// specialize std::hash with the bigNumOLD type
	template<> struct hash<bigNumOLD> {
		size_t operator()(const bigNumOLD& n) const {	// assumes no leading/trailing zeros
			hash<bigNumOLD::digitType> hasher;
			size_t h = 0;
			for (bigNumOLD::sizeType d = 0; d < n.size(); d++)
				h = h * 31 + hasher(n[d]);
			return h;
		}
	};
}



bigNumOLD::sizeType bigNumOLD::mPrecision = bigNumOLD::mDefaultPrecision;

/* ------------------------------------------------------ */
/*           Constructors + Special Member Fncs           */
/* -------------------------------------------------------*/

bigNumOLD::bigNumOLD():
	mDigits(new digitType[mDefaultSize]()), mSize(mDefaultSize), mNumDigits(1), mIntegerStart(0), mRepetendStart(0), mRepetendSize(0), mIsNegative(false), mIsInteger(true), mIsZero(true)
{ }
bigNumOLD::bigNumOLD(sizeType size, sizeType radixPos):
	mNumDigits(1), mRepetendStart(0), mRepetendSize(0), mIsNegative(false), mIsZero(true)
{
	if (size < 1) size = mDefaultSize;
	if (radixPos < 0) radixPos = 0;
	mIntegerStart = radixPos;
	mIsInteger = (radixPos == 0);
	mSize = size;
	mDigits = new digitType[size]();
}
bigNumOLD::bigNumOLD(std::string number) {
	if (number.length() == 0)
		throw std::length_error("bigNumOLD cannot be constructed with no digits");

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
	if (number.length() == 0) throw std::length_error("bigNumOLD cannot be constructed with no digits");
	mSize = number.length();
	mDigits = new digitType[mSize]();
	for (sizeType d = 0; d < mSize; d++) {
		if (number[d] < '0' || number[d] > '9')
			throw std::invalid_argument("Number contains an illegal character");
		mDigits[d] = number[(mSize - 1) - d] - '0';		// digits are stored little-endian
	}

	// Update state variables
	countDigits();
	mRepetendStart = 0;
	mRepetendSize = 0;
}
bigNumOLD::bigNumOLD(bigNumOLD &&other) noexcept:
	mDigits(other.mDigits), mSize(other.mSize), mNumDigits(other.mNumDigits), mIntegerStart(other.mIntegerStart), mRepetendStart(other.mRepetendStart), mRepetendSize(other.mRepetendSize), mIsNegative(other.mIsNegative), mIsInteger(other.mIsInteger), mIsZero(other.mIsZero)
{
	other.mDigits = nullptr;
}
bigNumOLD::bigNumOLD(const bigNumOLD &other):
	mSize(other.mSize), mNumDigits(other.mNumDigits), mIntegerStart(other.mIntegerStart), mRepetendStart(other.mRepetendStart), mRepetendSize(other.mRepetendSize), mIsNegative(other.mIsNegative), mIsInteger(other.mIsInteger), mIsZero(other.mIsZero)
{
	mDigits = new digitType[other.mSize]();
	for (sizeType d = 0; d < other.mSize; d++)
		mDigits[d] = other.mDigits[d];
}
bigNumOLD& bigNumOLD::operator=(const bigNumOLD &rhs) {
	if (&rhs == this) return *this;	// check for self-assignment
	if (rhs.mSize != mSize) {		// check if a new size is needed
			delete[] mDigits;
			mSize = rhs.mSize;
			mDigits = new digitType[rhs.mSize]();
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

	return *this;
}
bigNumOLD& bigNumOLD::operator=(bigNumOLD &&rhs) noexcept {
	if (&rhs == this) return *this;	// check for self-assignment
	mSize = rhs.mSize;
	mNumDigits = rhs.mNumDigits;
	mIntegerStart = rhs.mIntegerStart;
	mRepetendStart = rhs.mRepetendStart;
	mRepetendSize = rhs.mRepetendSize;
	mIsNegative = rhs.mIsNegative;
	mIsInteger = rhs.mIsInteger;
	mIsZero = rhs.mIsZero;

	delete[] mDigits;
	mDigits = rhs.mDigits;
	rhs.mDigits = nullptr;

	return *this;
}
bigNumOLD::~bigNumOLD() {
	delete[] mDigits;
}

/* ------------------------------------------------------ */
/*                   Utility + Getters                    */
/* -------------------------------------------------------*/

bool bigNumOLD::countDigits() { 
	// Count the number of significant digits (there is always at least 1 in the one's place)
	// Updates mNumDigits, mIsZero, mIsInteger

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
	} else mIsZero = false;
	mIsInteger = numFractionDigits > 0 ? false : true;

	return true;
}
bigNumOLD& bigNumOLD::shift(sizeType shiftAmt, ShiftDirection direction) {
	if (shiftAmt == 0 || mSize <= 0) return *this;

	// shift the position of the decimal point
	sizeType numNewDigits = 0;
	if (direction == ShiftDirection::RIGHT) {	// move decimal point left
		mIntegerStart += shiftAmt;
		numNewDigits = (mIntegerStart >= mSize) ? mIntegerStart - mSize + 1 : 0;
	} else {			// move decimal point right
		sizeType radixShiftAmt = (mIntegerStart > shiftAmt) ? shiftAmt : mIntegerStart;
		mIntegerStart -= radixShiftAmt;
		numNewDigits = shiftAmt - radixShiftAmt;
		if (mRepetendSize > 0) {
			while (mRepetendStart > 0 && (mRepetendStart + mRepetendSize) > mIntegerStart) mRepetendStart--;
			if (mRepetendStart + mRepetendSize > mIntegerStart) {	// need add'tl digits to keep the entire repetend fractional
				sizeType numExtraRepDigits = mRepetendStart + mRepetendSize - mIntegerStart;
				numNewDigits += numExtraRepDigits;
				mIntegerStart += numExtraRepDigits;
			}
		}
	}

	// add additional digits
	if (numNewDigits > 0) {
		sizeType newSize = mSize + numNewDigits;
		digitType *newData = new digitType[newSize]();	// new digits are zero-init'd

		if (direction == ShiftDirection::RIGHT) {
			for (sizeType i = 0; i < mSize; i++)
				newData[i] = mDigits[i];
			if (mRepetendSize > 0) {	// if the repetend ends with 0's, increase the start position ("slide" the rep over)
				sizeType i = mRepetendStart;
				while (newData[i] == 0 && i < (mRepetendStart + mRepetendSize)) i++;
				mRepetendStart = i;
			}
		} else {
			if (mRepetendSize > 0) {	// new digits beyond the lsb are an extension of the repetend
				for (sizeType i = 0, r = mRepetendStart + (mRepetendSize - 1); i < numNewDigits; i++) {
					sizeType index = (numNewDigits - 1) - i;
					newData[index] = mDigits[r];
					if (r == mRepetendStart) r += mRepetendSize - 1;
					else r--;
				}
			}
			for (sizeType i = numNewDigits, j = 0; j < mSize && i < newSize; j++, i++)
				newData[i] = mDigits[j];
		}

		delete[] mDigits;
		mDigits = newData;
		mSize = newSize;
	}

	countDigits();
	return *this;
}
//bigNumOLD& bigNumOLD::shift(int shift) {
//	// shift is positive for a left shift (equivalent to multiplying by 10^shift)
//	// shift is negative for a right shift (equivalent to dividing by 10^shift)
//
//	if (mSize <= 0) throw std::invalid_argument("Invalid object");
//	if (shift == 0 || mSize <= 0) return *this;
//	sizeType shiftAmt = (sizeType)std::abs(shift);
//
//	// shift the position of the decimal point
//	sizeType numNewDigits = 0;
//	if (shift < 0) {	// move decimal point left (right shift)
//		mIntegerStart += shiftAmt;
//		numNewDigits = (mIntegerStart >= mSize) ? mIntegerStart - mSize + 1 : 0;
//	} else {			// move decimal point right (left shift)
//		sizeType radixShiftAmt = (mIntegerStart > shiftAmt) ? shiftAmt : mIntegerStart;
//		mIntegerStart -= radixShiftAmt;
//		numNewDigits = shiftAmt - radixShiftAmt;
//		if (mRepetendSize > 0) {
//			while (mRepetendStart > 0 && (mRepetendStart + mRepetendSize) > mIntegerStart) mRepetendStart--;
//			if (mRepetendStart + mRepetendSize > mIntegerStart) {	// need add'tl digits to keep the entire repetend fractional
//				sizeType numExtraRepDigits = mRepetendStart + mRepetendSize - mIntegerStart;
//				numNewDigits += numExtraRepDigits;
//				mIntegerStart += numExtraRepDigits;
//			}
//		}
//	}
//
//	// add additional digits
//	if (numNewDigits > 0) {
//		sizeType newSize = mSize + numNewDigits;
//		digitType *newData = new digitType[newSize]();	// new digits are zero-init'd
//
//		if (shift < 0) {	// right shift
//			for (sizeType i = 0; i < mSize; i++)
//				newData[i] = mDigits[i];
//		} else {			// left shift
//			if (mRepetendSize > 0) {	// new digits beyond the lsb are an extension of the repetend
//				for (sizeType i = 0, r = mRepetendStart + (mRepetendSize - 1); i < numNewDigits; i++) {
//					sizeType index = (numNewDigits - 1) - i;
//					newData[index] = mDigits[r];
//					if (r == mRepetendStart) r += mRepetendSize - 1;
//					else r--;
//				}
//			}
//			for (sizeType i = numNewDigits, j = 0; j < mSize && i < newSize; j++, i++)
//				newData[i] = mDigits[j];
//		}
//
//		delete[] mDigits;
//		mDigits = newData;
//		mSize = newSize;
//	}
//
//	countDigits();
//	return *this;
//}
bool bigNumOLD::padMSB(sizeType numZeros) {	// every bigNumOLD will have at least one integer digit; no change to mIntegerStart
	if (numZeros <= 0) return false;
	sizeType newSize = mSize + numZeros;
	digitType *newData = new digitType[newSize]();
	for (sizeType i = 0; i < mSize; i++)	// copy old digits
		newData[i] = mDigits[i];
	delete[] mDigits;
	mDigits = newData;
	mSize = newSize;
	return true;
}
bool bigNumOLD::padLSB(sizeType numDigits) {
	if (numDigits <= 0) return false;

	sizeType newSize = mSize + numDigits;
	digitType *newData = new digitType[newSize]();	// zero-init new digits

	if (mRepetendSize > 0)	// new digits are added by extending the repetend
		for (sizeType i = 0, r = mRepetendStart + (mRepetendSize-1); i < numDigits; i++) {
			sizeType index = (numDigits-1) - i;
			newData[index] = mDigits[r];
			if (r == mRepetendStart) r += mRepetendSize-1;
			else r--;
		}
	for (sizeType i = numDigits, j = 0; i < newSize && j < mSize; i++, j++)	// copy old digits
		newData[i] = mDigits[j];

	delete[] mDigits;
	mDigits = newData;
	mSize = newSize;
	if (mRepetendSize > 0) mRepetendStart += numDigits;
	if (mIntegerStart > 0) mIntegerStart += numDigits;
	if (mIntegerStart == 0 || mRepetendSize > 0) mNumDigits += numDigits;
	return true;
}
bool bigNumOLD::removeZeros(bool trailingZerosAreSig) {
	// Remove leading zeros and trailing zeros after the decimal point (unless they are part of a repetend)
	if (mSize == 0) return false;
	sizeType end = mSize-1, start = 0;
	while (end > mIntegerStart && mDigits[end] == 0) end--;
	if (!trailingZerosAreSig)
		while (start < mIntegerStart && (mRepetendSize > 0 ? (start < mRepetendStart) : 1) && mDigits[start] == 0)
			start++;
	if (end != (mSize-1) || start != 0) {
		sizeType newSize = end - start + 1;
		digitType* newData = new digitType[newSize];
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
void bigNumOLD::print(sizeType firstNDigits /* = 0 */) const {
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

	for (sizeType i = 0; i < firstNDigits; i++) {
		sizeType index = end - i;
		if (mRepetendSize > 0 && index == (mRepetendStart + mRepetendSize-1)) std::cout << "(";
		std::cout << mDigits[index];
		if (mRepetendSize > 0 && index == mRepetendStart) {
			std::cout << ")";
			break;
		}
		if (index == mIntegerStart && i != (firstNDigits - 1)) std::cout << ".";

	}
}
void bigNumOLD::printAll() const {
	if (mSize == 0) {
		std::cout << "No digits found\n";
		return;
	}
	if (mIsNegative) std::cout << "-";
	for (sizeType i = 0; i < mSize; i++) {
		sizeType index = mSize - 1 - i;
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
void bigNumOLD::reset() {
	delete[] mDigits;
	mSize = mDefaultSize;
	mDigits = new digitType[mSize]();
	mNumDigits = 1;
	mIntegerStart = 0;
	mRepetendStart = 0;
	mRepetendSize = 0;
	mIsNegative = false;
	mIsInteger = true;
	mIsZero = true;
}
bigNumOLD abs(const bigNumOLD &n) {
	bigNumOLD a = n;
	a.mIsNegative = false;
	return a;
}
bigNumOLD operator-(const bigNumOLD &n) {
	bigNumOLD a = n;
	a.mIsNegative = !n.mIsNegative;
	return a;
}
bigNumOLD::sizeType bigNumOLD::numDigits() const { return mNumDigits; }
bigNumOLD::sizeType bigNumOLD::size() const { return mSize; }
bigNumOLD::sizeType bigNumOLD::repetendPeriod() const { return mRepetendSize; }
bigNumOLD::digitType& bigNumOLD::operator[](sizeType d) {
	if (d < 0 || d >= mSize) throw std::range_error("Index out of range");
	return mDigits[mSize - 1 - d];
}
const bigNumOLD::digitType& bigNumOLD::operator[](sizeType d) const {
	if (d < 0 || d >= mSize) throw std::range_error("Index out of range");
	return mDigits[mSize - 1 - d];
};
bigNumOLD::operator int() const{
	// truncates any fraction digits

	if (*this > bigNumOLD(INT_MAX) || *this < bigNumOLD(INT_MIN)) return 0;

	sizeType end = mSize - 1;
	while (end > mIntegerStart && end > 0 && mDigits[end] == 0) end--;

	std::stringstream ss;
	for (sizeType d = 0; d < mNumDigits; d++)
		ss << mDigits[end - d];

	int result;
	ss >> result;
	if (mIsNegative) result = -result;

	return result;
}

/* ------------------------------------------------------ */
/*				  Comparison Operators                    */
/* -------------------------------------------------------*/

// update with repetend
bool bigNumOLD::operator==(const bigNumOLD &rhs) const {	// Test for equality ignores leading and trailing zeros
	const bigNumOLD &lhs = *this;

	if (lhs.mIsZero && rhs.mIsZero) return true;
	if (lhs.mIsNegative != rhs.mIsNegative) return false;
	if (lhs.mIsInteger != rhs.mIsInteger) return false;
	if (lhs.mNumDigits != rhs.mNumDigits) return false;

	// Skip trailing zeros
	sizeType lhsStartIndex = 0, rhsStartIndex = 0;
	while (lhsStartIndex < lhs.mIntegerStart && lhs.mDigits[lhsStartIndex] == 0) lhsStartIndex++;
	while (rhsStartIndex < rhs.mIntegerStart && rhs.mDigits[rhsStartIndex] == 0) rhsStartIndex++;
	if ((lhs.mIntegerStart - lhsStartIndex) != (rhs.mIntegerStart - rhsStartIndex)) return false;	// make sure decimal point is in the same place

	for (sizeType i = lhsStartIndex, j = rhsStartIndex; i < lhs.mNumDigits; i++, j++)
		if (lhs.mDigits[i] != rhs.mDigits[j]) return false;

	return true;
}
bool bigNumOLD::operator!=(const bigNumOLD &rhs) const {
	return !(*this == rhs);
}
// update with repetend
bool bigNumOLD::operator>(const bigNumOLD &rhs) const {
	const bigNumOLD &lhs = *this;

	// Check if one number is + and one is -
	if (!lhs.mIsNegative && rhs.mIsNegative) return true;
	if (lhs.mIsNegative && !rhs.mIsNegative) return false;

	// Check if one number has more integer digits
	sizeType lhsMSIntDigitPos = lhs.mSize - 1;
	sizeType rhsMSIntDigitPos = rhs.mSize - 1;
	while (lhsMSIntDigitPos > lhs.mIntegerStart && lhs.mDigits[lhsMSIntDigitPos] == 0) lhsMSIntDigitPos--;
	while (rhsMSIntDigitPos > rhs.mIntegerStart && rhs.mDigits[rhsMSIntDigitPos] == 0) rhsMSIntDigitPos--;
	if ((lhsMSIntDigitPos - lhs.mIntegerStart) != (rhsMSIntDigitPos - rhs.mIntegerStart))
		return (lhsMSIntDigitPos - lhs.mIntegerStart > rhsMSIntDigitPos - rhs.mIntegerStart);

	// Compare the digits
	sizeType l = lhsMSIntDigitPos, r = rhsMSIntDigitPos;
	sizeType i = 0;
	for (; i < lhs.mNumDigits && i < rhs.mNumDigits; i++) {
		if (lhs.mDigits[l-i] > rhs.mDigits[r-i]) return true;
		if (lhs.mDigits[l-i] < rhs.mDigits[r-i]) return false;
	}
	
	// If the lhs has more fraction digits, check if any are non-zero
	while (i < lhs.mNumDigits) {
		if (lhs.mDigits[l-i] > 0) return true;
		i++;
	}

	// lhs <= rhs
	return false;
}
bool bigNumOLD::operator<(const bigNumOLD &rhs) const {
	const bigNumOLD &lhs = *this;

	// Check if the numbers are equal before using operator>
	if (lhs == rhs) return false;

	return !(lhs > rhs);
}
bool bigNumOLD::operator>=(const bigNumOLD &rhs) const {
	const bigNumOLD &lhs = *this;

	// Check if the numbers are equal before using operator>
	if (lhs == rhs) return true;

	return (lhs > rhs);
}
bool bigNumOLD::operator<=(const bigNumOLD &rhs) const {
	const bigNumOLD &lhs = *this;

	// Check if the numbers are equal before using operator>
	if (lhs == rhs) return true;

	return !(lhs > rhs);
}

/* -------------------------------- */
/*             Addition             */
/* ---------------------------------*/

void bigNumOLD::absValAdd(const bigNumOLD &rhs) {
	// rhs is assumed to have a matching sign

	int off_F = (int)mIntegerStart - (int)rhs.mIntegerStart;
	int off_I = (int)(mSize - mIntegerStart) - (int)(rhs.mSize - rhs.mIntegerStart);
	sizeType off_F_amt = (sizeType)(off_F < 0 ? -off_F : off_F);
	sizeType off_I_amt = (sizeType)(off_I < 0 ? -off_I : off_I);

	if (off_F < 0 || off_I < 0) {	// new data must be allocated
		sizeType newSize = mSize + (off_F<0 ? off_F_amt : 0) + (off_I<0 ? off_I_amt : 0) + 1;
		digitType *sum = new digitType[newSize]();

		digitType carry = 0;
		sizeType s = 0, r = 0, l = 0;
		
		// Copy over the digits from the longer fraction
		if (off_F < 0) {
			for (; r < off_F_amt; r++, s++)
				sum[s] = rhs.mDigits[r];
		} else {
			for (; l < off_F_amt; l++, s++)
				sum[s] = mDigits[l];
		}

		// Add the digits both numbers have in common
		for (; l < mSize && r < rhs.mSize && s < newSize-1; l++, r++, s++) {
			sum[s] = mDigits[l] + rhs.mDigits[r] + carry;
			if (sum[s] > 9) {
				sum[s] -= 10;
				carry = 1;
			} else carry = 0;
		}

		// Copy over the digits from the longer integer (+ any carry)
		if (off_I < 0) {
			for (; r < rhs.mSize && s < newSize-1; r++, s++) {
				sum[s] = rhs.mDigits[r] + carry;
				if (sum[s] > 9) {
					sum[s] -= 10;
					carry = 1;
				} else carry = 0;
			}
		} else {
			for (; l < mSize && s < newSize-1; l++, s++) {
				sum[s] = mDigits[l] + carry;
				if (sum[s] > 9) {
					sum[s] -= 10;
					carry = 1;
				} else carry = 0;
			}
		}
		sum[s] = carry;

		delete[] mDigits;
		mSize = newSize;
		mDigits = sum;
		mIntegerStart = off_F < 0 ? rhs.mIntegerStart : mIntegerStart;
	} else {						// add rhs directly to *this
		digitType carry = 0;
		sizeType l = off_F_amt;

		// Add digits of rhs to *this
		for (sizeType r = 0; r < rhs.mSize && l < mSize; r++, l++) {
			mDigits[l] += rhs.mDigits[r] + carry;
			if (mDigits[l] > 9) {
				mDigits[l] -= 10;
				carry = 1;
			} else carry = 0;
		}

		// Propogate any carry
		while (carry && l < mSize) {
			mDigits[l] += carry;
			if (mDigits[l] > 9) mDigits[l] -= 10;
			else carry = 0;
			l++;
		}
		if (carry) {	// result of addition requires an extra digit
			digitType *sum = new digitType[mSize+1]();
			for (sizeType i = 0; i < mSize; i++) sum[i] = mDigits[i];
			sum[mSize] = carry;
			delete[] mDigits;
			mDigits = sum;
			mSize++;
		}
	}

	countDigits();
}
bigNumOLD& bigNumOLD::operator+=(const bigNumOLD &rhs) {
	if (mIsNegative == rhs.mIsNegative)
		absValAdd(rhs);
	else
		absValSubtract(rhs);
	return *this;
}
bigNumOLD operator+(bigNumOLD lhs, const bigNumOLD &rhs) {
	lhs += rhs;
	return lhs;
}
bigNumOLD& bigNumOLD::operator++() {
	if (mIsNegative) {
		digitType carry = 1;
		sizeType i = mIntegerStart;
		while (carry && i < mSize) {
			mDigits[i] = (9 - mDigits[i]) + carry;
			if (mDigits[i] > 9) mDigits[i] -= 10;
			else carry = 0;
			i++;
		}
		if (carry) {
			mIsNegative = !mIsNegative;
			sizeType j = mIntegerStart;
			while (carry) {
				mDigits[j] += carry;
				if (mDigits[j] > 9) mDigits[j] -= 10;
				else carry = 0;
			}
		} else {
			for (sizeType j = mIntegerStart; j < i; j++)
				mDigits[j] = 9 - mDigits[j];
		}
	} else {
		digitType carry = 1;
		sizeType i = mIntegerStart;
		while (carry && i < mSize) {
			mDigits[i] += carry;
			if (mDigits[i] > 9) mDigits[i] -= 10;
			else carry = 0;
			i++;
		}
		if (carry) {
			digitType *newData = new digitType[mSize + 1];
			for (sizeType i = 0; i < mSize; i++)
				newData[i] = mDigits[i];
			newData[mSize] = carry;
			delete[] mDigits;
			mDigits = newData;
			mSize++;
		}
	}
	countDigits();
	return *this;
}
bigNumOLD bigNumOLD::operator++(int) {
	bigNumOLD returnVal(*this);
	operator++();
	return returnVal;
}

/* ------------------------------------------------------- */
/*            Subtraction (Method of complements)          */
/* --------------------------------------------------------*/

void bigNumOLD::absValSubtract(const bigNumOLD &rhs) {
	// rhs is assumed to have a matching sign

	int off_F = (int)mIntegerStart - (int)rhs.mIntegerStart;
	int off_I = (int)(mSize - mIntegerStart) - (int)(rhs.mSize - rhs.mIntegerStart);
	sizeType off_F_amt = (sizeType)(off_F < 0 ? -off_F : off_F);
	sizeType off_I_amt = (sizeType)(off_I < 0 ? -off_I : off_I);

	digitType *diff = mDigits;
	sizeType diffSize = mSize;
	bool newDataAllocated = false;
	if (off_F < 0 || off_I < 0) {	// new data must be allocated
		diffSize = mSize + (off_F<0 ? off_F_amt : 0) + (off_I<0 ? off_I_amt : 0);
		digitType *newData = new digitType[diffSize]();
		diff = newData;
		newDataAllocated = true;
	}

	// Take the 9's complement and add the rhs
	
	sizeType d = 0, l = 0, r = 0;
	digitType carry = 0;

	// Copy over the digits from the longer fraction
	if (off_F < 0) {
		for (; r < off_F_amt; d++, r++) {
			diff[d] = 9 + rhs.mDigits[r] + carry;
			if (diff[d] > 9) {
				diff[d] -= 10;
				carry = 1;
			} else carry = 0;
		}
	} else if (off_F > 0) {
		for (; l < off_F_amt; d++, l++)
			diff[d] = 9 - mDigits[l];
	}
	// Sum the digits both numbers have in common
	for (; l < mSize && r < rhs.mSize && d < diffSize; l++, r++, d++) {
		diff[d] = (9 - mDigits[l]) + rhs.mDigits[r] + carry;
		if (diff[d] > 9) {
			diff[d] -= 10;
			carry = 1;
		} else carry = 0;
	}
	// Copy over the digits from the longer integer (+ any carry)
	if (off_I < 0) {
		for (; r < rhs.mSize && d < diffSize; r++, d++) {
			diff[d] = 9 + rhs.mDigits[r] + carry;
			if (diff[d] > 9) {
				diff[d] -= 10;
				carry = 1;
			} else carry = 0;
		}
	} else if (off_I > 0) {
		for (; l < mSize && d < diffSize; l++, d++) {
			diff[d] = (9 - mDigits[l]) + carry;
			if (diff[d] > 9) {
				diff[d] -= 10;
				carry = 1;
			} else carry = 0;
		}
	}
	if (carry) {	// result of subtraction is negative, rhs > *this
		// add back the carry to the least significant digit (end-arround carry)
		sizeType i = 0;
		while (carry && i < diffSize) {
			diff[i] += carry;
			if (diff[i] > 9) diff[i] -= 10;
			else carry = 0;
			i++;
		}
		mIsNegative = !mIsNegative;
	} else {		// result of subtraction is positive, *this >= rhs
		// take the 9's complement
		for (sizeType i = 0; i < diffSize; i++)
			diff[i] = 9 - diff[i];
	}

	if (newDataAllocated) {
		delete[] mDigits;
		mSize = diffSize;
		mDigits = diff;
		mIntegerStart = off_F < 0 ? rhs.mIntegerStart : mIntegerStart;
	}

	countDigits();
}
bigNumOLD& bigNumOLD::operator-=(const bigNumOLD &rhs) {
	if (mIsNegative == rhs.mIsNegative)
		absValSubtract(rhs);
	else
		absValAdd(rhs);
	return *this;
}
bigNumOLD operator-(bigNumOLD lhs, const bigNumOLD &rhs) {
	lhs -= rhs;
	return lhs;
}
bigNumOLD& bigNumOLD::operator--() {
	if (mIsNegative) {
		digitType carry = 1;
		sizeType i = mIntegerStart;
		while (carry && i < mSize) {
			mDigits[i] += carry;
			if (mDigits[i] > 9) mDigits[i] -= 10;
			else carry = 0;
			i++;
		}
		if (carry) {
			digitType *newData = new digitType[mSize + 1];
			for (sizeType i = 0; i < mSize; i++)
				newData[i] = mDigits[i];
			newData[mSize] = carry;
			delete[] mDigits;
			mDigits = newData;
			mSize++;
		}
	} else {
		digitType carry = 1;
		sizeType i = mIntegerStart;
		while (carry && i < mSize) {
			mDigits[i] = (9 - mDigits[i]) + carry;
			if (mDigits[i] > 9) mDigits[i] -= 10;
			else carry = 0;
			i++;
		}
		if (carry) {
			mIsNegative = !mIsNegative;
			sizeType j = mIntegerStart;
			while (carry) {
				mDigits[j] += carry;
				if (mDigits[j] > 9) mDigits[j] -= 10;
				else carry = 0;
			}
		} else {
			for (sizeType j = mIntegerStart; j < i; j++)
				mDigits[j] = 9 - mDigits[j];
		}
	}
	countDigits();
	return *this;
}
bigNumOLD bigNumOLD::operator--(int) {
	bigNumOLD returnVal(*this);
	operator--();
	return returnVal;
}

/* --------------------------------------------------- */
/*                  Multiplication                     */
/* ----------------------------------------------------*/

void bigNumOLD::absValIntegerLongMultiplication(const bigNumOLD &rhs) {
	// assumes *this and rhs are both integers and have matching signs

	sizeType prodSize = mSize + rhs.mSize;
	digitType *prod = new digitType[prodSize]();

	digitType sum = 0;
	for (sizeType p = 0; p < prodSize-1; p++) {
		for (sizeType r = (p < mSize) ? 0 : p-mSize+1; r <= p && r < rhs.mSize; r++)
			sum += mDigits[p-r] * rhs.mDigits[r];
		prod[p] = sum % 10;
		sum /= 10;
	}
	prod[prodSize-1] = sum % 10;

	delete[] mDigits;
	mDigits = prod;
	mSize = prodSize;
	countDigits();
}
std::pair<bigNumOLD, bigNumOLD> bigNumOLD::splitAt(sizeType index) const {
	if (index == 0) return {bigNumOLD("0"), bigNumOLD(*this)};
	if (index >= mSize) return {bigNumOLD(*this), bigNumOLD("0")};
	bigNumOLD low(index, 0), high(mSize-index, 0);
	sizeType i = 0;
	for (; i < index; i++)
		low.mDigits[i] = mDigits[i];
	for (sizeType j = 0; i < mSize; i++, j++)	// digit at index belongs to high
		high.mDigits[j] = mDigits[i];
	low.countDigits();
	high.countDigits();
	return {low, high};
}
bigNumOLD bigNumOLD::karatsuba(const bigNumOLD& x, const bigNumOLD& y) const {
	// assumes x and y are both integers and have matching signs

	// Base case: For operands under 256 digits, use long multiplication
	if (x.mSize < 256 || y.mSize < 256) {
		bigNumOLD result = x;
		result.absValIntegerLongMultiplication(y);
		return result;
	}

	// Choose m
	sizeType n = (x.mSize > y.mSize) ? x.mSize : y.mSize;
	sizeType m = n / 2;

	// Split both bigNums into a low and high part separated at position m
	std::pair<bigNumOLD, bigNumOLD> xP = x.splitAt(m);
	std::pair<bigNumOLD, bigNumOLD> yP = y.splitAt(m);
	bigNumOLD &x0 = xP.first, &x1 = xP.second;
	bigNumOLD &y0 = yP.first, &y1 = yP.second;

	/*
		Calculate Karatsuba equations, recursively call this function to perform the multiplications
		z0 = (x0)*(y0)
		z2 = (x1)*(y1)
		z1 = (x0 + x1)*(y0 + y1) - z0 - z2
	*/
	bigNumOLD z0 = karatsuba(x0, y0);
	bigNumOLD z2 = karatsuba(x1, y1);
	bigNumOLD z1 = karatsuba((x0 + x1), (y0 + y1)) - z0 - z2;

	/*
		Compute the result using z0, z1, z2
		result = ((10^(2*m)) * z2) + ((10^m) * z1) + z0
	*/
	bigNumOLD result = z2.shift(2*m, ShiftDirection::LEFT) + z1.shift(m, ShiftDirection::LEFT) + z0;

	return result;
}
bigNumOLD& bigNumOLD::operator*=(const bigNumOLD &rhs) {

	// Check for zero operands
	if (mIsZero) return *this;
	else if (rhs.mIsZero) {
		reset();
		return *this;
	}

	if (mSize < 256 || rhs.mSize < 256) {
		absValIntegerLongMultiplication(rhs);
		mIntegerStart = mIntegerStart + rhs.mIntegerStart;
		mIsNegative = mIsNegative ^ rhs.mIsNegative;
	} else {
		bigNumOLD result = karatsuba(*this, rhs);
		result.mIntegerStart = mIntegerStart + rhs.mIntegerStart;
		result.mIsNegative = mIsNegative ^ rhs.mIsNegative;
		*this = result;
	}

	countDigits();
	return *this;
}
bigNumOLD operator*(bigNumOLD lhs, const bigNumOLD &rhs) {
	lhs *= rhs;
	return lhs;
}


/* ------------------------------------------------------- */
/*             Division (Newton-Raphson Method)            */
/* --------------------------------------------------------*/

//redo
bigNumOLD bigNumOLD::round(const bigNumOLD &n, sizeType precision) const {
	if (precision == 0)
		throw std::length_error("bigNumOLD cannot be constructed with no digits");

	if (precision >= n.mNumDigits) {
		bigNumOLD result = n;		// make a copy of n
		return result;
	}

	bigNumOLD result(n.mNumDigits, 0);

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
		bigNumOLD result1(n.mNumDigits + 1, 0);
		sizeType i;
		for (i = 0; i < result.mSize; i++)
			result1.mDigits[i] = result.mDigits[i];
		result1.mDigits[i] = carry;
		result = result1;
	}

	// Check if the rounded number is 0 and update mIsNegative and mIntegerStart
	result.mIntegerStart = n.mIntegerStart;
	result.mIsNegative = n.mIsNegative;
	result.countDigits();
	
	return result;
}

//bigNumOLD bigNumOLD::newtonRaphsonDivide(const bigNumOLD &lhs, const bigNumOLD &rhs) const {
//
//	// Newton-Raphson Division
//	// Q = N/D
//	// 1. Calculate an estimate X0 for the reciprocal (1/D) of the divisor
//	// 2. Compute successively more accurate estimates X1, X2, ... of the reciprocal
//	// 3. Compute the quotient by multiplying the numerator by the reciprocal Xn
//
//	bigNumOLD N = lhs;
//	bigNumOLD D = rhs;
//	digitType precision = 50;							// CONSIDER CHANGING TO SOME OTHER VALUE
//
//														// Scale denominator to be in [0,1)
//	int e = D.mNumDigits - D.mIntegerStart;				// FIX THIS FOR NUMBERS < 0
//	bigNumOLD Dnorm = D;
//	Dnorm.mIntegerStart += e;						// FIX: DECIMALPOINTPOS MAY BE UPDATED BEYOND MSIZE
//
//														// Scale numerator by the same amount
//	bigNumOLD Nnorm = N;
//	Nnorm.mIntegerStart += e;						// FIX: DECIMALPOINTPOS MAY BE UPDATED BEYOND MSIZE
//
//														// Calculate an initial guess X0 for 1/Dnorm		// FIX THIS FOR ANY LENGTH DNORM
//														// X0 = A - B(Dnorm)								// FIX CONSTANTS MUST BE ROUNDED NOT TRUNCATED
//	std::string a = divConstantA.substr(0, Dnorm.mNumDigits + 2);	// Calculate the constants to the same
//	std::string b = divConstantB.substr(0, Dnorm.mNumDigits + 2);	// precision as Dnorm
//	bigNumOLD A = round(bigNumOLD(a), Dnorm.mNumDigits);
//	bigNumOLD B = round(bigNumOLD(b), Dnorm.mNumDigits);
//	bigNumOLD X0 = A - (B * Dnorm);
//
//	// Refine the initial guess by iterating through:
//	// Xn = Xn-1 + Xn-1 * (1 - Dnorm * Xn-1)^
//	bigNumOLD Xn = X0, Xnminus1 = X0;
//	bigNumOLD one("1");
//	bigNumOLD two("2");
//	for (int i = 0; i < 5; i++) {						// FIX THIS WITH THE CORRECT # OF ITERATIONS
//		Xnminus1 = Xn;
//		Xn = Xnminus1 + (Xnminus1 * (one - (Dnorm * Xnminus1)));	// FIX THIS BY LIMITING PRECISION OF EACH   <--------- this next
//		Xn = absValAdd(Xnminus1, karatsuba(Xnminus1, absValSubtract(one, karatsuba(Dnorm, Xnminus1))));
//		//Xn = Xnminus1 * (two - (Dnorm * Xnminus1));
//	}															// MULTIPLICATION TO FINAL (?) PRECISION
//
//																// Calculate the quotient by multiplying Nnorm by the final iteration
//	bigNumOLD result = Nnorm * Xn;
//
//	return result;
//}


/* ------------------------------------------------------- */
/*             Division (Long Division)                    */
/* --------------------------------------------------------*/

int bigNumOLD::trimToSignificand() {
	// Removes all leading and trailing zeros and converts the number to an integer
	// Returns how many digits were shifted (positive for a left shift, negative for a right shift)

	if (mSize == 0) return 0;
	sizeType end = mSize - 1, start = 0;
	while (mDigits[end] == 0 && end > 0) end--;
	while (mDigits[start] == 0 && start < end && (mRepetendSize > 0 ? (start < mRepetendStart) : true)) start++;
	if (end != (mSize - 1) || start != 0) {
		sizeType newSize = end - start + 1;
		digitType* newData = new digitType[newSize];
		for (sizeType i = 0, j = start; i < newSize; i++, j++)
			newData[i] = mDigits[j];
		delete[] mDigits;
		mDigits = newData;
		mSize = newSize;
		mNumDigits = newSize;
	}
	int shiftAmt = (int)mIntegerStart - (int)start;
	mIntegerStart = 0;
	if (mRepetendSize > 0) mRepetendStart -= start;
	return shiftAmt;
}
void bigNumOLD::multiplyByDigit(digitType d) {
	if (d == 1) return;
	digitType carry = 0;
	for (sizeType i = 0; i < mSize; i++) {
		mDigits[i] = mDigits[i] * d + carry;
		carry = mDigits[i] / 10;
		mDigits[i] %= 10;
	}
	if (carry != 0) {	// result of multiplication requires 1 extra digit
		digitType *newData = new digitType[mSize + 1];
		for (sizeType i = 0; i < mSize; i++)
			newData[i] = mDigits[i];
		newData[mSize] = carry;
		delete[] mDigits;
		mDigits = newData;
		mSize++;
		mNumDigits++;
	}
}
void bigNumOLD::divideByDigit(digitType d) {
	if (d == 0) throw std::invalid_argument("Division by zero");
	if (d == 1) return;

	// Make space for up to 9 digits of precision
	sizeType precision = mSize + 9;
	digitType *newData = new digitType[precision]();

	// Divide each digit into d
	sizeType p;
	digitType r = 0;
	for (p = 0; p < mSize; p++) {
		newData[precision - 1 - p] = (10 * r + mDigits[mSize - 1 - p]) / d;
		r = (10 * r + mDigits[mSize - 1 - p]) % d;
	}

	// Divide each remainder into d until the quotient terminates or repeats
	std::unordered_map<digitType, sizeType> remainders;	// keep track of remainders to detect a repetend
	remainders.emplace(r, p - 1);
	for (; p < precision; p++) {
		// If the remainder is 0, the result of the division terminates
		if (r == 0) break;

		newData[precision - 1 - p] = (10 * r) / d;
		r = (10 * r) % d;

		// Check if this remainder has been seen before --> indicates the existence of a repetend
		auto it = remainders.find(r);
		if (it != remainders.end()) {
			mRepetendSize = p - it->second;
			mRepetendStart = precision - 1 - p;
			break;
		} else remainders.emplace(r, p);
	}
	delete[] mDigits;
	mDigits = newData;
	mSize = precision;
	mIntegerStart += 9;
	removeZeros();
	countDigits();
}

bigNumOLD bigNumOLD::newDivide(bigNumOLD u, bigNumOLD v, DivisionMode mode, sizeType precision) {
	if (v.mIsZero) throw std::invalid_argument("Division by zero");

	// Reduce u and v to integers
	int scaling = u.trimToSignificand() - v.trimToSignificand();
	if (u.mSize < v.mSize) {	// make sure u has more digits than v
		sizeType shiftAmt = v.mSize - u.mSize;
		scaling += shiftAmt;
		u.shift(shiftAmt, ShiftDirection::LEFT);
	}
	sizeType quotientShiftAmt = std::abs(scaling);
	ShiftDirection quotientShiftDir = scaling > 0 ? ShiftDirection::RIGHT : ShiftDirection::LEFT;

	// Simpler algorithm when v is 1 digit
	if (v.mSize == 1) {
		u.divideByDigit(v.mDigits[0]);
		if (quotientShiftAmt) u.shift(quotientShiftAmt, quotientShiftDir);
		return u;
	}

	// Normalize the dividend and divisor so that the first digit of the divisor is >= 5
	digitType d = 10 / (v[0] + 1);
	sizeType preNormUSize = u.mSize;
	u.multiplyByDigit(d);
	v.multiplyByDigit(d);
	if (u.mSize == preNormUSize) u.padMSB(1);		// Ensure u has one extra leading digit

	sizeType n = v.mSize + 1;		// size of partial dividend
	sizeType preScaleIntegerStart = u.mSize - v.mSize;	// # of integer digits in the (pre-scaled) quotient
	sizeType postScaleIntegerStart = scaling > (preScaleIntegerStart-1) ? 0 : (preScaleIntegerStart-1)-scaling;
	sizeType quotientSize = precision+1;
	if (quotientSize < preScaleIntegerStart + quotientShiftAmt) quotientSize = preScaleIntegerStart + quotientShiftAmt + 1;
	bigNumOLD quotient(quotientSize, quotientSize-preScaleIntegerStart);
	std::unordered_map<bigNumOLD, sizeType> remainders;	// keep track of remainders to detect a repetend
	sizeType repStart = 0, repSize = 0;

	for (sizeType p = 0, i = (u.mSize-n); p < quotientSize && p < precision+1; p++) {
		
		// Partial dividend consists of the digits u.mDigits[i] (lsb) to u.mDigits[i+n-1] (msb)

		// Make a guess for q and refine it so it is at most 1 too large
		digitType msb = i+n-1;
		digitType v0 = v.mDigits[v.mSize-1], v1 = v.mDigits[v.mSize-2];
		digitType q = (u.mDigits[msb] == v0) ? 9 : (10*u.mDigits[msb] + u.mDigits[msb-1]) / v0;
		while (q*v1 > (10 * (10*u.mDigits[msb] + u.mDigits[msb-1] - q*v0) + u.mDigits[msb-2])) q--;

		// Compute the partial quotient by subtracting q*v from the partial dividend in u in-place
		bigNumOLD product = v;
		product.multiplyByDigit(q);
		if (product.mSize != n) product.padMSB(n - product.mSize);

		// (subtraction done using method of complements)
		digitType carry = 0;
		for (sizeType j = 0; j < n; j++) {		// Complement the minuend and add the subtrahend
			u.mDigits[i+j] = 9 - u.mDigits[i+j] + product.mDigits[j] + carry;
			if (u.mDigits[i+j] > 9) {
				carry = 1;
				u.mDigits[i+j] -= 10;
			} else carry = 0;
		}
		if (carry == 0) {						// Complement to get the partial quotient
			for (sizeType j = 0; j < n; j++)
				u.mDigits[i+j] = 9 - u.mDigits[i+j];
		} else if (carry == 1) {				// Result is negative - q was 1 too large
			// "End-around carry" - add the carry to the LSB to get the difference (a negative value)
			for (sizeType j = 0; j < n; j++) {
				u.mDigits[i+j] += carry;
				if (u.mDigits[i+j] > 9) u.mDigits[i+j] -= 10;
				else break;
			}
			// Add back v to get the correct partial quotient (done by subtracting the partial quotient from v since the partial quotient was negative)
			carry = 0;
			for (sizeType j = 0; j < (n-1); j++) {
				u.mDigits[i+j] += 9 - v.mDigits[j] + carry;
				if (u.mDigits[i+j] > 9) {
					carry = 1;
					u.mDigits[i+j] -= 10;
				} else carry = 0;
			}
			u.mDigits[i+n-1] = 9;				// Carry will always be 0 for the last digit
			for (sizeType j = 0; j < n; j++)
				u.mDigits[i+j] = 9 - u.mDigits[i+j];
			q--;
		}

		// Set this digit of the final quotient
		quotient.mDigits[quotientSize-1-p] = q;

		if (p == postScaleIntegerStart && mode == DivisionMode::EUCLIDQUOTIENT || mode == DivisionMode::EUCLIDREMAINDER) break;
		else if (p > postScaleIntegerStart) {
		
			// Check if the quotient has terminated
			bool isZero = true;
			for (sizeType j = 0; j < n; j++)
				if (u.mDigits[j] != 0) {
					isZero = false;
					break;
				}
			if (isZero) break;

			// Check for a repetend
			if (repSize == 0) {
				auto it = remainders.find(u);
				if (it != remainders.end()) {	// If this remainder has been seen before there is a repetend
					repSize = p - it->second;
					repStart = quotientSize-p;
				} else remainders.emplace(u, p);
			}
		}
		
		// Update the partial dividend's index
		if (i > 0) i--;
		else {
			// Shift the remainder left by 1 (multiply by 10)
			for (sizeType j = n-1; j > 0; j--)
				u.mDigits[j] = u.mDigits[j-1];
			u.mDigits[0] = 0;
		}
	}

	if (mode == DivisionMode::EUCLIDREMAINDER) {
		u.divideByDigit(d);
		u.removeZeros();
		u.countDigits();
		return u;
	}
	quotient.mRepetendSize = repSize;
	quotient.mRepetendStart = repStart;
	if (quotientShiftAmt) quotient.shift(quotientShiftAmt, quotientShiftDir);
	quotient.removeZeros(true);
	quotient.countDigits();
	return quotient;
}

bool bigNumOLD::precisionDivide(const bigNumOLD &rhs, sizeType precision) {
	// Divides *this by rhs up to the specified precision 
	// Returns true if a terminating or repeating quotient is found, false otherwise

	bigNumOLD &u = *this;
	bigNumOLD v = rhs;		// make a copy, will be modified

	// Convert u and v to integers and remove leading/trailing zeros
	int scaling = v.trimToSignificand() - u.trimToSignificand();

	// Make sure u has at least as many digits as v
	if (u.mSize < v.mSize) {
		sizeType shiftAmt = v.mSize - u.mSize;
		scaling -= shiftAmt;
		u.shift(shiftAmt, ShiftDirection::LEFT);
	}

	sizeType quotientShiftAmt = std::abs(scaling);
	ShiftDirection quotientShiftDir = scaling < 0 ? ShiftDirection::RIGHT : ShiftDirection::LEFT;

	// Simpler algorithm if v is 1 digit
	if (v.mSize == 1) {
		u.divideByDigit(v.mDigits[0]);
		if (quotientShiftAmt) shift(quotientShiftAmt, quotientShiftDir);
		return true;
	}

	// ______________________________________________________________________________
	// REVIEW THIS

	sizeType n = v.mSize + 1;				// size of partial dividend/quotient
	sizeType m = u.mSize - v.mSize + 1;		// size of integer quotient
	//if (precision < m) precision = m;
	//if (precision < m + quotientScalingFactor) precision = m + quotientScalingFactor;		// <---------
	if (precision < m + quotientShiftAmt) precision = m + quotientShiftAmt;
	digitType *quotient = new digitType[precision]();
	std::unordered_map<bigNumOLD, sizeType> remainders;	// keep track of remainders to detect a repetend
	bool returnVal = false;

	// __________________________________________________________________________________



	// Normalize the dividend and divisor so that the first digit of the divisor is >= 5
	digitType d = 10 / (v[0] + 1);
	sizeType uSize = u.mSize;
	u.multiplyByDigit(d);
	v.multiplyByDigit(d);
	if (u.mSize == uSize) u.padMSB(1);		// Ensure u has one extra leading digit

	/*
		Division of integers u and v up to specified precision
		- For each iteration, divide the partial dividend (in u) by v, in-place, to obtain one digit (q) of the final quotient and a partial quotient (in u)
		- After the digits of the dividend have been exhaused (when i = m-1), continue performing division iterations up to the specified precision, each time generating a new remainder and multiplying it by 10
	*/
	for (sizeType p = 0, i = 0; p < precision; p++, (i<m-1) ? i++ : i) {

		// Partial dividend/quotient consists of the digits u[i] to u[i+n-1]

		// Make a guess for q and refine it so it is at most 1 too large
		digitType q = (u[i] == v[0]) ? 9 : (u[i] * 10 + u[i+1]) / v[0];
		while ((v[1] * q) >(10 * (u[i] * 10 + u[i+1] - q * v[0]) + u[i+2])) q--;

		// Compute the partial quotient by subtracting q*v from the partial dividend in u in-place
		bigNumOLD product = v;
		product.multiplyByDigit(q);
		if (product.mSize != n) product.padMSB(n - product.mSize);
		sizeType l = m-1-i;	// Index of partial dividend's least significant digit

		// (subtraction done using method of complements)
		digitType carry = 0;
		for (sizeType j = 0; j < n; j++) {		// Complement the minuend and add the subtrahend
			u.mDigits[l + j] = 9 - u.mDigits[l + j] + product.mDigits[j] + carry;
			if (u.mDigits[l + j] > 9) {
				carry = 1;
				u.mDigits[l + j] -= 10;
			} else carry = 0;
		}
		if (carry == 0) {						// Complement to get the partial quotient
			if (i == m-1) {	// partial quotient is a remainder, shift to multiply by 10 for next iteration
				for (sizeType j = 0; j < (n-1); j++)
					u.mDigits[n-1-j] = 9 - u.mDigits[n-2-j];
				u.mDigits[0] = 0;
			} else {
				for (sizeType j = 0; j < n; j++)
					u.mDigits[l + j] = 9 - u.mDigits[l + j];
			}
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
				u.mDigits[l+j] += 9 - v.mDigits[j] + carry;
				if (u.mDigits[l+j] > 9) {
					carry = 1;
					u.mDigits[l+j] -= 10;
				} else carry = 0;
			}
			u.mDigits[l+n-1] = 9;				// Carry will always be 0 for the last digit
			if (i == m - 1) {	// partial quotient is a remainder, shift to multiply by 10 for next iteration
				for (sizeType j = 0; j < (n - 1); j++)
					u.mDigits[n - 1 - j] = 9 - u.mDigits[n - 2 - j];
				u.mDigits[0] = 0;
			} else {
				for (sizeType j = 0; j < n; j++)
					u.mDigits[l + j] = 9 - u.mDigits[l + j];
			}

			q--;
		}

		// Set this digit of the final quotient
		quotient[precision-1-p] = q;

		// If the division generated a remainder, check for a terminating or repeating quotient
		if (i == m - 1) {
			// Remainder is stored in u; if it is 0, the result of the division terminates
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

			// If this remainder has been seen before there is a repetend
			auto it = remainders.find(u);
			if (it != remainders.end()) {
				mRepetendSize = p - it->second;
				mRepetendStart = precision-1-p;
				returnVal = true;
				break;
			} else remainders.emplace(u, p);
		}
	}

	delete[] mDigits;
	mDigits = quotient;
	mSize = precision;
	mIntegerStart = precision - m;
	if (quotientShiftAmt) shift(quotientShiftAmt, quotientShiftDir);
	removeZeros();
	countDigits();
	return returnVal;
}
std::pair<bigNumOLD, bigNumOLD> euclideanDivide(bigNumOLD u, bigNumOLD v) {
	// Returns a quotient and remainder pair, quotient will be an integer

	using sizeType = bigNumOLD::sizeType;
	using digitType = bigNumOLD::digitType;
	using ShiftDirection = bigNumOLD::ShiftDirection;

	if (v.mIsZero) throw std::invalid_argument("Division by zero");

	

	// Convert u and v to integers and remove leading/trailing zeros (quotient is unaffected but remainder will need to be unscaled)
	//sizeType remShiftAmt = u.mIntegerStart > v.mIntegerStart ? u.mIntegerStart : v.mIntegerStart;
	//int scaling = v.trimToSignificand() - u.trimToSignificand();
	

	// ___________________________________________________________________________________
	// Clean this up

	// Convert u and v to integers (does not affect quotient, but will scale remainder)
	sizeType remShiftAmt = u.mIntegerStart > v.mIntegerStart ? u.mIntegerStart : v.mIntegerStart;
	int scalingFactor = u.mIntegerStart - v.mIntegerStart;
	u.mIntegerStart = 0;
	v.mIntegerStart = 0;
	u.removeZeros();
	v.removeZeros();
	if (scalingFactor) {
		sizeType scalingAmt = (sizeType)(scalingFactor < 0 ? -scalingFactor : scalingFactor);
		if (scalingFactor < 0) u.padLSB(scalingAmt);
		else if (scalingFactor > 0) v.padLSB(scalingAmt);
	}
	if (u.mSize < v.mSize) return {bigNumOLD(0), u};

	// ____________________________________________________________________________________

	// Algorithm for euclidean division of u by a single digit, quotient is stored in u and remainder is returned
	auto euclideanDivideUByDigit = [&](digitType d) -> digitType {
		if (d == 0) throw std::invalid_argument("Division by zero");
		digitType r = 0;
		for (sizeType i = 0; i < u.mSize; i++) {
			digitType q = 10*r + u.mDigits[(u.mSize-1)-i];
			u.mDigits[(u.mSize-1)-i] = q / d;
			r = q % d;
		}
		return r;
	};
	if (v.mSize == 1) {
		bigNumOLD remainder(euclideanDivideUByDigit(v.mDigits[0]));
		if (remShiftAmt) remainder.shift(remShiftAmt, ShiftDirection::RIGHT);
		u.countDigits();
		return {u, remainder};
	}

	sizeType n = v.mSize + 1;				// size of partial dividend
	sizeType m = u.mSize - v.mSize + 1;		// size of quotient
	bigNumOLD quotient(m, 0);

	// Normalize the dividend and divisor so that the first digit of the divisor is >= 5
	digitType d = 10 / (v[0] + 1);
	sizeType uSize = u.mSize;
	u.multiplyByDigit(d);
	v.multiplyByDigit(d);
	if (u.mSize == uSize) u.padMSB(1);		// Ensure u has one extra leading digit

	/*
		Euclidean division of integers u and v (where u.mSize > v.mSize)
		- For each iteration, divide the partial dividend (in u) by v, in-place, to obtain one digit (q) of the final quotient and a partial quotient (in u)
		- The final partial quotient (in u) is the remainder
	*/
	for (sizeType i = 0; i < m; i++) {

		// Partial dividend/quotient consists of the digits u[i] to u[i+n-1]
		
		// Make a guess for q by dividing the first two digits of the partial dividend u[i]u[i+1] by v[0] and adjusting if the guess is too high
		digitType q = (u[i] == v[0]) ? 9 : (u[i] * 10 + u[i+1]) / v[0];	
		while ((v[1]*q) > (10*(u[i]*10 + u[i+1] - q*v[0]) + u[i+2])) q--;

		// Compute the partial quotient by subtracting q*v from the partial dividend in-place
		if (q == 0) {
			quotient[i] = q;
			continue;
		}
		bigNumOLD product = v;
		product.multiplyByDigit(q);
		if (product.mSize != n) product.padMSB(n - product.mSize);
		sizeType l = m-1-i;		// Index of partial dividend's least significant digit

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
			u.mDigits[l+n-1] = 9;				// Carry will always be 0 for the last digit
			for (sizeType j = 0; j < n; j++)	// Complementing gives the partial quotient
				u.mDigits[l + j] = 9 - u.mDigits[l + j];
			q--;
		}

		// Set this digit of the final quotient
		quotient[i] = q;
	}

	// Remainder is in u, denormalize and unscale
	euclideanDivideUByDigit(d);
	if (remShiftAmt) u.shift(remShiftAmt, ShiftDirection::RIGHT);
	u.removeZeros();
	u.countDigits();
	quotient.countDigits();
	return {quotient, u};
}
bigNumOLD& bigNumOLD::operator/=(const bigNumOLD &rhs) {

	// Check for zero operands
	if (rhs.mIsZero) throw std::invalid_argument("Divison by zero");
	if (mIsZero) return *this;

	precisionDivide(rhs);
	mIsNegative = mIsNegative ^ rhs.mIsNegative;

	return *this;
}
bigNumOLD operator/(bigNumOLD lhs, const bigNumOLD &rhs) {
	lhs /= rhs;
	return lhs;
}
bigNumOLD& bigNumOLD::operator%=(const bigNumOLD &rhs) {

	// Check for zero operands
	if (rhs.mIsZero) throw std::invalid_argument("Division by zero");
	if (mIsZero) return *this;

	*this = euclideanDivide(*this, rhs).second;

	return *this;
}
bigNumOLD operator%(bigNumOLD lhs, const bigNumOLD &rhs) {
	lhs %= rhs;
	return lhs;
}

/* -------------------------------- */
/*          Exponentiation          */
/* ---------------------------------*/

bigNumOLD& bigNumOLD::operator^=(const bigNumOLD &power) {
	// currently works on integer powers

	if (power.mIsZero) {
		*this = 1;
		return *this;
	}
	if (power.mIntegerStart != 0) return *this;

	// convert power into a long long
	constexpr int64_t max = std::numeric_limits<int64_t>::max();
	constexpr int64_t min = std::numeric_limits<int64_t>::min();
	if (power > max || power < min) return *this;
	sizeType end = power.mSize - 1;
	while (end > power.mIntegerStart && power.mDigits[end] == 0) end--;
	std::stringstream ss;
	for (sizeType d = 0; d < power.mNumDigits; d++)
		ss << power.mDigits[end - d];
	int64_t powerInt;
	ss >> powerInt;

	// convert powerInt into binary
	std::string binary("");
	do {
		binary = (powerInt % 2 != 0 ? "1" : "0") + binary;
	} while (powerInt /= 2);
	binary.erase(binary.begin());

	// perform successive multiplications
	bigNumOLD a = *this;
	for (auto b : binary) {
		*this *= *this;
		if (b == '1') *this *= a;
	}

	if (power.mIsNegative) *this = 1 / *this;

	removeZeros();
	countDigits();
	return *this;
}
bigNumOLD operator^(bigNumOLD lhs, const bigNumOLD &rhs) {
	lhs ^= rhs;
	return lhs;
}


/* -------------------------------- */
/*             Factorial		    */
/* ---------------------------------*/

bigNumOLD factorial(const bigNumOLD &n) {
	
	// Check the number is a non-negative integer
	if (!n.mIsInteger || n.mIsNegative) return bigNumOLD("0");

	bigNumOLD result = n;
	bigNumOLD i = n - 1;
	while (i != 1) {
		result *= i;
		i--;
	}
	return result;
}

























// old absValAdd + absValSubtract

/*
bigNumOLD bigNumOLD::absValAdd(const bigNumOLD &a, const bigNumOLD &b) const {
	const bigNumOLD& longFrac = (a.mIntegerStart > b.mIntegerStart) ? a : b;	// operand with more fraction digits
	const bigNumOLD& shortFrac = (&longFrac == &a) ? b : a;					// operand with fewer fraction digits (indexed with an offset)
	const bigNumOLD& longInt = (a.mSize-a.mIntegerStart) > (b.mSize-b.mIntegerStart) ? a : b;		// operand with more integer digits
	const bigNumOLD& shortInt = (&longInt == &a) ? b : a;						// operand with fewer integer digits

	sizeType sumSize = (longInt.mSize - longInt.mIntegerStart) + longFrac.mIntegerStart + 1;
	bigNumOLD sum(sumSize, longFrac.mIntegerStart);

	sizeType offset = longFrac.mIntegerStart - shortFrac.mIntegerStart;
	sizeType highestCommonDigit = (sumSize-1) - ((longInt.mSize - longInt.mIntegerStart) - (shortInt.mSize - shortInt.mIntegerStart));

	digitType carry = 0;
	sizeType s = 0;
	for (; s < offset; s++)
		sum.mDigits[s] = longFrac.mDigits[s];
	for (; s < highestCommonDigit; s++) {
		sum.mDigits[s] = longFrac.mDigits[s] + shortFrac.mDigits[s-offset] + carry;
		if (sum.mDigits[s] > 9) {
			sum.mDigits[s] -= 10;
			carry = 1;
		} else carry = 0;
	}
	sizeType l = s - (&longInt == &shortFrac ? offset : 0);
	for (; l < longInt.mSize && s < sumSize-1; s++, l++) {
		sum.mDigits[s] = longInt.mDigits[l] + carry;
		if (sum.mDigits[s] > 9) {
			sum.mDigits[s] -= 10;
			carry = 1;
		} else carry = 0;
	}
	sum.mDigits[s] = carry;
	sum.countDigits();

	return sum;
}
*/

/*
bigNumOLD bigNumOLD::absValSubtract(const bigNumOLD& lhs, const bigNumOLD& rhs) const {

	// Minuend - subtrahend = difference

	// Make sure the minuend is > subtrahend
	const bigNumOLD& min = (lhs > rhs) ? lhs : rhs;
	const bigNumOLD& sub = (&min == &lhs) ? rhs : lhs;
	bool differenceIsNeg = (&min == &rhs);

	const bigNumOLD& longFrac = (min.mIntegerStart > sub.mIntegerStart) ? min : sub;	// operand with more fraction digits
	const bigNumOLD& shortFrac = (&longFrac == &min) ? sub : min;						// operand with fewer fraction digits (indexed with an offset)
	sizeType offset = longFrac.mIntegerStart - shortFrac.mIntegerStart;

	sizeType differenceSize = (min.mSize-min.mIntegerStart) + longFrac.mIntegerStart;	// num of integer digits + num fraction digits
	bigNumOLD difference(differenceSize, longFrac.mIntegerStart);

	// Take the 9's complement of the minuend
	sizeType minOffset = (&min == &shortFrac) ? offset : 0;
	sizeType d = 0;
	for (; d < minOffset; d++)
		difference.mDigits[d] = 9;
	for (sizeType m = 0; m < min.mSize && d < differenceSize; m++, d++)
		difference.mDigits[d] = 9 - min.mDigits[m];

	// Add the subtrahend
	digitType carry = 0;
	d = (&sub == &shortFrac) ? offset : 0;
	for (sizeType s = 0; s < sub.mSize && d < differenceSize; s++, d++) {
		difference.mDigits[d] += sub.mDigits[s] + carry;
		if (difference.mDigits[d] > 9) {
			difference.mDigits[d] -= 10;
			carry = 1;
		} else carry = 0;
	}
	while (carry && d < differenceSize) {
		difference.mDigits[d] += carry;
		if (difference.mDigits[d] > 9)
			difference.mDigits[d] -= 10;
		else carry = 0;
		d++;
	}

	// Take the 9's complement to get the difference
	for (d = 0; d < differenceSize; d++)
		difference.mDigits[d] = 9 - difference.mDigits[d];

	difference.mIsNegative = differenceIsNeg;
	difference.countDigits();

	return difference;
}
*/


// old operator*=

/*
bigNumOLD& bigNumOLD::operator*=(const bigNumOLD &rhs) {
	if (mIsZero) return *this;
	else if (rhs.mIsZero) {
		reset();
		return *this;
	}
	if (mNumDigits == 1) {
		bigNumOLD result = rhs;
		result.multiplyByDigit(mDigits[0]);
		result.mIsNegative = mIsNegative ^ rhs.mIsNegative;
		result.countDigits();
		*this = result;
		return *this;
	} else if (rhs.mNumDigits == 1) {
		multiplyByDigit(rhs.mDigits[0]);
		mIsNegative = mIsNegative ^ rhs.mIsNegative;
		countDigits();
		return *this;
	}

	if (mSize < 256 || rhs.mSize < 256) {
		absValIntegerLongMultiplication(rhs);
		mIntegerStart = mIntegerStart + rhs.mIntegerStart;
	} else {
		// Perform karatsuba multiplication (operates on integers)
		bigNumOLD result = karatsuba(*this, rhs);

		// The result contains a number of fraction digits equal to the sum of a and b's fraction digits
		sizeType lhsStart = 0, rhsStart = 0;
		while (mDigits[lhsStart] == 0 && lhsStart < mIntegerStart) lhsStart++;
		while (rhs.mDigits[rhsStart] == 0 && rhsStart < rhs.mIntegerStart) rhsStart++;
		sizeType lhsFractionSize = mIntegerStart - lhsStart;
		sizeType rhsFractionSize = rhs.mIntegerStart - rhsStart;
		result.mIntegerStart = lhsFractionSize + rhsFractionSize;

		// If either operand's most significant digits begin with 0, the result will be missing leading zeros
		if (result.mIntegerStart >= result.mSize)
			result.padMSB(result.mIntegerStart - result.mSize + 1);
		result.mIsNegative = mIsNegative ^ rhs.mIsNegative;
		result.countDigits();
		*this = result;
	}

	return *this;
}
*/


// Old subtraction algo using method of complements (method 2 on wiki)

//bigNumOLD bigNumOLD::absValSubtract(bigNumOLD &lhs, bigNumOLD &rhs) const {
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
//	bigNumOLD ninesComplement(rhs.mSize);
//	for (sizeType i = 0; i < rhs.mSize; i++)
//		ninesComplement[i] = 9 - rhs.mDigits[i];
//	ninesComplement.mIntegerStart = rhs.mIntegerStart;
//	ninesComplement.countDigits();
//
//	// Add the 9's complement of the subtrahend and the minuend
//	bigNumOLD sum = absValAdd(lhs, ninesComplement);
//
//	bigNumOLD result;
//
//	// If the sum contains an extra digit, "carry" it to the LSB by adding 1 and dropping the MSB
//	if (sum.mNumDigits > lhs.mNumDigits && sum.mNumDigits > rhs.mNumDigits) {	// result is positive
//		// "end-around carry"
//
//		result = bigNumOLD(lhs.mSize);		// set the result's size to the size of an operand, dropping the MSB of the sum
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
//		bigNumOLD sumComplement(sum.mSize);
//		for (sizeType i = 0; i < sum.mSize; i++)
//			sumComplement[i] = 9 - sum.mDigits[i];
//		result = sumComplement;
//		result.mIsNegative = true;
//	}
//	result.mIntegerStart = sum.mIntegerStart;
//	result.countDigits();
//	return result;
//}



// Add'tl check in setKaratsubaDecimalPoint. Should not be needed now that every bigNumOLD has an integer part?

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


// Old remove zeros fncs

//// Removes leading, integer zeros
//bool bigNumOLD::removeLeadingZeros() { // Updates mSize, mDigits
//	if (mSize == 0) return false;
//
//	sizeType end = mSize - 1;
//	while (end > mIntegerStart && mDigits[end] == 0) end--;
//	if (end != (mSize - 1)) {
//		sizeType newSize = end + 1;
//		digitType* newData = new digitType[newSize];
//		for (sizeType i = 0; i < newSize; i++)
//			newData[i] = mDigits[i];
//		delete[] mDigits;
//		mDigits = newData;
//		mSize = newSize;
//		return true;
//	}
//
//	return false;
//}
//
//// Removes trailing, fractional zeros
//bool bigNumOLD::removeTrailingZeros() { // Updates mSize, mDigits, mIntegerStart
//	if (mSize == 0) return false;
//
//	sizeType start = 0;
//	while (start < mIntegerStart && (mRepetendSize > 0 ? (start < mRepetendStart) : 1) && mDigits[start] == 0) start++;
//	if (start != 0) {
//		sizeType newSize = mSize - start;
//		digitType* newData = new digitType[newSize];
//		for (sizeType i = 0, j = start; i < newSize; i++, j++)
//			newData[i] = mDigits[j];
//		delete[] mDigits;
//		mDigits = newData;
//		mSize = newSize;
//		mIntegerStart -= start;
//		if (mRepetendSize > 0) mRepetendStart -= start;
//		return true;
//	}
//
//	return false;
//}

// Old single digit division fncs (moved to lambdas)

//// Returns a quotient and remainder
//bigNumOLD bigNumOLD::divideByDigit(digitType d, bigNumOLD& remainder) const {
//
//	if (d == 0) throw std::invalid_argument("Division by zero");
//
//	bigNumOLD quotient(mSize);
//	digitType r = 0;
//
//	for (sizeType i = 0; i < mSize; i++) {
//		quotient.mDigits[(mSize - 1) - i] = (10 * r + mDigits[(mSize - 1) - i]) / d;
//		r = (10 * r + mDigits[(mSize - 1) - i]) % d;
//	}
//
//	remainder = bigNumOLD(std::to_string(r));
//
//	quotient.countDigits();
//	remainder.countDigits();
//
//	return quotient;
//}





// Old shift fncs


//// Equivalent to multiplying the number by 10^shiftAmt, returns true if digits were added, false otherwise
//bool bigNumOLD::shiftLeft(sizeType shiftAmt) {		// updates mIntegerStart, mSize, mDigits, mNumDigits
//	if (shiftAmt <= 0) return false;
//
//	bool returnVal = false;
//
//	// shift the position of the decimal point first
//	if (mIntegerStart > 0) {
//		sizeType radixShiftAmt = (mIntegerStart > shiftAmt) ? shiftAmt : mIntegerStart;
//		mIntegerStart -= radixShiftAmt;
//		shiftAmt -= radixShiftAmt;
//	}
//
//	// add additional digits
//	if (shiftAmt > 0) {
//		sizeType numNewDigits = shiftAmt + mRepetendSize;
//		sizeType newSize = mSize + numNewDigits;
//		digitType* newData = new digitType[newSize];
//		sizeType i = 0;
//
//		// add new digits
//		if (mRepetendSize > 0) {	// extend the repetend
//			for (sizeType r = mRepetendStart + (mRepetendSize - 1); i < numNewDigits; i++) {
//				sizeType index = (numNewDigits - 1) - i;
//				newData[index] = mDigits[r];
//				if (r == mRepetendStart) r += mRepetendSize - 1;
//				else r--;
//			}
//		} else {						// pad zeros
//			for (; i < numNewDigits; i++)
//				newData[i] = 0;
//		}
//		// copy old digits
//		for (sizeType j = 0; j < mSize; j++, i++)
//			newData[i] = mDigits[j];
//
//		// update members
//		delete[] mDigits;
//		mDigits = newData;
//		mSize = newSize;
//		if (mRepetendSize > 0) {	// additional fraction digits were added for the repetend
//			mRepetendStart = 0;
//			mIntegerStart += mRepetendSize;
//		}
//		returnVal = true;
//	} else if (mRepetendSize > 0 && (mRepetendStart + mRepetendSize) > mIntegerStart) {
//
//		// repetend contains integer digits, adjust so it is entirely fractional
//
//		// adjust the beginning of the repetend
//		while (mRepetendStart > 0 && (mRepetendStart + mRepetendSize) > mIntegerStart) mRepetendStart--;
//
//		// add additional digits
//		if ((mRepetendStart + mRepetendSize) > mIntegerStart) {
//			sizeType numNewDigits = (mRepetendStart + mRepetendSize) - mIntegerStart;
//			sizeType newSize = mSize + numNewDigits;
//			digitType* newData = new digitType[newSize];
//			sizeType i = 0;
//
//			// add new digits
//			for (sizeType r = mRepetendStart + (mRepetendSize - 1); i < numNewDigits; i++) {
//				sizeType index = (numNewDigits - 1) - i;
//				newData[index] = mDigits[r];
//				if (r == mRepetendStart) r += mRepetendSize - 1;
//				else r--;
//			}
//
//			// copy old digits
//			for (sizeType j = 0; j < mSize; j++, i++)
//				newData[i] = mDigits[j];
//
//			// update members
//			delete[] mDigits;
//			mDigits = newData;
//			mSize = newSize;
//			mRepetendStart = 0;
//			mIntegerStart += numNewDigits;
//
//			returnVal = true;
//		}
//	}
//	countDigits();
//	return returnVal;
//}
//
//// Equivalent to dividing the number by 10^shiftAmt, returns true if digits were added, false otherwise
//bool bigNumOLD::shiftRight(sizeType shiftAmt) {
//	if (shiftAmt == 0) return false;
//
//	// shift the radix point
//	mIntegerStart += shiftAmt;
//
//	// add additional zeros
//	if (mIntegerStart >= mSize) {
//		sizeType numZeros = mIntegerStart - mSize + 1;
//		sizeType newSize = mSize + numZeros;
//		digitType* newData = new digitType[newSize];
//
//		// Copy the digits over, starting from the LSB
//		sizeType i = 0;
//		for (; i < mSize; i++)
//			newData[i] = mDigits[i];
//
//		// Pad the zeros beyond the MSB
//		for (; i < newSize; i++)
//			newData[i] = 0;
//
//		delete[] mDigits;
//		mDigits = newData;
//		mSize = newSize;
//	}
//	if (mRepetendSize > 0) mRepetendStart += shiftAmt;
//
//	countDigits();
//
//	return true;
//}




// old add/sub

//bigNumOLD bigNumOLD::absValAdd(bigNumOLD &a, bigNumOLD &b) const {
//
//	// Make both integer parts the same length
//	sizeType aIntDigits = a.mSize - a.mIntegerStart;
//	sizeType bIntDigits = b.mSize - b.mIntegerStart;
//	if (aIntDigits > bIntDigits) b.padMSB(aIntDigits - bIntDigits);
//	else if (aIntDigits < bIntDigits) a.padMSB(bIntDigits - aIntDigits);
//
//	// Make both fraction parts the same length
//	sizeType aFracDigits = a.mIntegerStart;
//	sizeType bFracDigits = b.mIntegerStart;
//	if (aFracDigits > bFracDigits) b.padLSB(aFracDigits - bFracDigits, true);
//	else if (aFracDigits < bFracDigits) a.padLSB(bFracDigits - aFracDigits, true);
//
//	// The sum may contain at most 1 additional digit
//	sizeType resultSize = a.mSize + 1;
//	bigNumOLD result(resultSize);
//	
//	// Add the digits, starting from the LSB
//	digitType carry = 0;
//	sizeType d;
//	for (d = 0; d < a.mSize; d++) {
//		result.mDigits[d] = a.mDigits[d] + b.mDigits[d] + carry;
//		if (result.mDigits[d] > 9) {
//			result.mDigits[d] -= 10;
//			carry = 1;
//		} else carry = 0;
//	}
//	if (carry == 1) result.mDigits[d] = carry;		// the sum contains an extra digit
//
//	// Update the decimal point position
//	result.mIntegerStart = a.mIntegerStart;
//	result.countDigits();
//
//	return result;
//}

// redo with indexing instead of reallocation
//bigNumOLD bigNumOLD::absValSubtract(bigNumOLD &lhs, bigNumOLD &rhs) const {
//
//	// Minuend - subtrahend = difference
//
//	// Make sure the minuend is > subtrahend
//	bigNumOLD& min = (lhs > rhs) ? lhs : rhs;
//	bigNumOLD& sub = (min == lhs) ? rhs : lhs;
//	bool differenceIsNeg = (min == rhs);
//
//	// Make both integer parts the same length
//	sizeType minIntDigits = min.mSize - min.mIntegerStart;
//	sizeType subIntDigits = sub.mSize - sub.mIntegerStart;
//	if (minIntDigits > subIntDigits) sub.padMSB(minIntDigits - subIntDigits);
//	else if (minIntDigits < subIntDigits) min.padMSB(subIntDigits - minIntDigits);
//
//	// Make both fraction parts the same length
//	sizeType minFracDigits = min.mIntegerStart;
//	sizeType subFracDigits = sub.mIntegerStart;
//	if (minFracDigits > subFracDigits) sub.padLSB(minFracDigits - subFracDigits, true);
//	else if (minFracDigits < subFracDigits) min.padLSB(subFracDigits - minFracDigits, true);
//	
//	// Calculate the 9's complement of the minuend
//	bigNumOLD result(min.mSize);
//	for (sizeType i = 0; i < result.mSize; i++)
//		result.mDigits[i] = 9 - min.mDigits[i];
//
//	// Add the subtrahend
//	digitType carry = 0;
//	for (sizeType d = 0; d < result.mSize; d++) {
//		result.mDigits[d] += sub.mDigits[d] + carry;
//		if (result.mDigits[d] > 9) {
//			carry = 1;
//			result.mDigits[d] -= 10;
//		} else carry = 0;
//	}
//
//	// Calculate the 9's complement
//	for (sizeType i = 0; i < result.mSize; i++)
//		result.mDigits[i] = 9 - result.mDigits[i];
//
//	// Update state
//	result.mIntegerStart = min.mIntegerStart;
//	result.mIsNegative = differenceIsNeg;
//	result.countDigits();
//	
//	return result;
//}