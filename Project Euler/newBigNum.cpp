#include "stdafx.h"
#include "newBigNum.h"
#include <string>
#include <limits>	// std::digits10
#include <numeric>	// std::lcm


/* ------------------------------------------------------ */
/*                Special Member Functions                */
/* -------------------------------------------------------*/

newBigNum::numArrayType::numArrayType():
	digits(new digitType[mDefaultSize]()), size(mDefaultSize), lsb(0), msb(0), isPositive(true)
{ }
newBigNum::numArrayType::numArrayType(sizeType sz) :
	lsb(0), msb(0), isPositive(true)
{
	if (sz < 1) sz = mDefaultSize;
	size = sz;
	digits = new digitType[size]();
}
newBigNum::numArrayType::numArrayType(const numArrayType &other):
	digits(new digitType[other.size]()), size(other.size), lsb(other.lsb), msb(other.msb), isPositive(other.isPositive)
{
	for (indexType i = 0; i < other.size; i++)
		digits[i] = other.digits[i];
}
newBigNum::numArrayType& newBigNum::numArrayType::operator=(const numArrayType &other) {
	if (&other == this) return *this;
	if (size != other.size) {
		delete[] digits;
		size = other.size;
		digits = new digitType[other.size]();
	}
	for (indexType i = 0; i < other.size; i++)
		digits[i] = other.digits[i];
	lsb = other.lsb;
	msb = other.msb;
	isPositive = other.isPositive;
	return *this;
}
newBigNum::numArrayType::numArrayType(numArrayType &&other) noexcept:
	digits(other.digits), size(other.size), lsb(other.lsb), msb(other.msb), isPositive(other.isPositive)
{
	other.digits = nullptr;
}
newBigNum::numArrayType& newBigNum::numArrayType::operator=(numArrayType &&other) noexcept {
	if (&other == this) return *this;
	delete[] digits;
	digits = other.digits;
	size = other.size;
	lsb = other.lsb;
	msb = other.msb;
	isPositive = other.isPositive;
	other.digits = nullptr;
	return *this;
}
newBigNum::numArrayType::~numArrayType() {
	delete[] digits;
}

// Constructors
newBigNum::newBigNum():
	mSignificand(), mExponent(), mRepLSB(-1), mRepMSB(-1)
{ }
newBigNum::newBigNum(sizeType sigSize, sizeType expSize):
	mRepLSB(-1), mRepMSB(-1)
{
	if (sigSize < 1) sigSize = mDefaultSize;
	if (expSize < 1) expSize = mDefaultSize;
	mSignificand = numArrayType(sigSize);
	mExponent = numArrayType(expSize);
}
newBigNum::newBigNum(std::string number) {
	if (number.length() == 0) throw std::length_error("BigNum cannot be constructed with no digits");
	
	// Check if the number is negative
	bool isPos = true;
	if (number[0] == '-') {
		isPos = false;
		number.erase(0, 1);
	}

	// Check if the number contains a decimal point
	indexType decimalPointPos = number.length();
	for (size_t i = 0; i < number.length(); i++) {
		if (number[i] == '.' || number[i] == ',') {
			decimalPointPos = i;
			number.erase(i, 1);
			break;
		}
	}

	// Read the digits
	if (number.length() == 0) throw std::length_error("BigNum cannot be constructed with no digits");
	size_t msbIndex = 0;
	while (number[msbIndex] == '0' && msbIndex < number.length()-1) msbIndex++;
	size_t stringLength = number.length() - msbIndex;
	if (stringLength > mMaxDigits) throw std::length_error("Overflow error in string constructor");
	sizeType sigSize = (stringLength > mDefaultSize) ? stringLength : mDefaultSize;
	mSignificand = numArrayType(sigSize);
	indexType i = 0;
	for (indexType j = number.length()-1; i < sigSize && j >= msbIndex; i++, j--) {
		if (number[j] < '0' || number[j] > '9')
			throw std::invalid_argument("Number contains an invalid character");
		mSignificand.digits[i] = number[j] - '0';	// digits are stored little-endian
	}
	mSignificand.lsb = 0;
	mSignificand.msb = i-1;
	mSignificand.isPositive = isPos;

	// Set the exponent
	indexType exp = decimalPointPos - msbIndex - 1;
	if (mSignificand.lsb == mSignificand.msb && mSignificand.digits[mSignificand.msb] == 0) exp = 0;	// the number 0 has an exp of 0
	isPos = (exp >= 0);
	if (exp < 0) exp = -exp;
	sizeType numExpDigits = (exp == 0) ? 1 : static_cast<sizeType>(std::log10(exp)) + 1;
	sizeType expSize = (numExpDigits > mDefaultSize) ? numExpDigits : mDefaultSize;
	mExponent = numArrayType(expSize);
	for (i = 0; i < numExpDigits; i++)
		mExponent.digits[i] = static_cast<indexType>(exp/std::pow(10, i)) % 10;
	mExponent.lsb = 0;
	mExponent.msb = i-1;
	mExponent.isPositive = isPos;

	mRepLSB = -1;
	mRepMSB = -1;
}
//newBigNum::~newBigNum() {
//	delete[] mSignificand.digits;
//	delete[] mExponent.digits;
//}
//newBigNum::newBigNum(const newBigNum &other):
//	mSignificand({
//		new digitType[other.mSignificand.size](),
//		other.mSignificand.size,
//		other.mSignificand.lsb,
//		other.mSignificand.msb,
//		other.mSignificand.isPositive
//	}),
//	mExponent({
//		new digitType[other.mExponent.size](),
//		other.mExponent.size,
//		other.mExponent.lsb,
//		other.mExponent.msb,
//		other.mExponent.isPositive
//	}),
//	mRepLSB(other.mRepLSB), mRepMSB(other.mRepMSB)
//{
//	for (indexType i = 0; i < other.mSignificand.size; i++)
//		mSignificand.digits[i] = other.mSignificand.digits[i];
//	for (indexType i = 0; i < other.mExponent.size; i++)
//		mExponent.digits[i] = other.mExponent.digits[i];
//}
//newBigNum::newBigNum(newBigNum &&other) noexcept:
//	mSignificand({
//		other.mSignificand.digits,
//		other.mSignificand.size,
//		other.mSignificand.lsb,
//		other.mSignificand.msb,
//		other.mSignificand.isPositive
//	}),
//	mExponent({
//		other.mExponent.digits,
//		other.mExponent.size,
//		other.mExponent.lsb,
//		other.mExponent.msb,
//		other.mExponent.isPositive
//	}),
//	mRepLSB(other.mRepLSB), mRepMSB(other.mRepMSB)
//{
//	other.mSignificand.digits = nullptr;
//	other.mExponent.digits = nullptr;
//}
//newBigNum& newBigNum::operator=(const newBigNum &rhs) {
//	if (&rhs == this) return *this;	// check for self-assignment
//	if (mSignificand.size != rhs.mSignificand.size) {
//		delete[] mSignificand.digits;
//		mSignificand.size = rhs.mSignificand.size;
//		mSignificand.digits = new digitType[rhs.mSignificand.size]();
//	}
//	if (mExponent.size != rhs.mExponent.size) {
//		delete[] mExponent.digits;
//		mExponent.size = rhs.mExponent.size;
//		mExponent.digits = new digitType[rhs.mExponent.size]();
//	}
//	for (indexType i = 0; i < rhs.mSignificand.size; i++)
//		mSignificand.digits[i] = rhs.mSignificand.digits[i];
//	for (indexType i = 0; i < rhs.mExponent.size; i++)
//		mExponent.digits[i] = rhs.mExponent.digits[i];
//	mSignificand.lsb = rhs.mSignificand.lsb;
//	mSignificand.msb = rhs.mSignificand.msb;
//	mSignificand.isPositive = rhs.mSignificand.isPositive;
//	mExponent.lsb = rhs.mExponent.lsb;
//	mExponent.msb = rhs.mExponent.msb;
//	mExponent.isPositive = rhs.mExponent.isPositive;
//	mRepLSB = rhs.mRepLSB;
//	mRepMSB = rhs.mRepMSB;
//	return *this;
//}
//newBigNum& newBigNum::operator=(newBigNum &&rhs) noexcept {
//	if (&rhs == this) return *this;	// check for self-assignment
//	delete[] mSignificand.digits;
//	mSignificand = rhs.mSignificand;
//	rhs.mSignificand.digits = nullptr;
//	delete[] mExponent.digits;
//	mExponent = rhs.mExponent;
//	rhs.mExponent.digits = nullptr;
//	mRepLSB = rhs.mRepLSB;
//	mRepMSB = rhs.mRepMSB;
//	return *this;
//}

/* ------------------------------------------------------ */
/*                       Utility                          */
/* -------------------------------------------------------*/

bool newBigNum::numArrayType::toInteger(indexType &out) const {
	// Returns true if the number is small enough to fit in an indexType variable
	indexType numDigits = msb - lsb + 1;
	constexpr indexType maxNumDigits = std::numeric_limits<indexType>::digits10;
	if (numDigits <= maxNumDigits) {
		out = 0;
		for (indexType i = lsb, e = 1; i <= msb && i < size; i++, e *= 10)
			out += e * digits[i];
		if (!isPositive) out = -out;
		return true;
	}
	return false;
}
newBigNum::digitType& newBigNum::operator[](indexType i) {
	if (i > mSignificand.size || i > (mSignificand.msb - mSignificand.lsb)) throw std::range_error("Index out of range");
	return mSignificand.digits[mSignificand.msb - i];
}
const newBigNum::digitType& newBigNum::operator[](indexType i) const {
	if (i > mSignificand.size || i > (mSignificand.msb - mSignificand.lsb)) throw std::range_error("Index out of range");
	return mSignificand.digits[mSignificand.msb - i];
}
void newBigNum::printFloat() const {
	if (mSignificand.size == 0 || mSignificand.lsb < 0 || mSignificand.msb < mSignificand.lsb || mExponent.size == 0 || mExponent.lsb < 0 || mExponent.msb < mExponent.lsb) {
		std::cerr << "No digits found\n";
		return;
	}

	// Print the significand
	indexType numSigDigits = mSignificand.msb - mSignificand.lsb + 1;
	if (!mSignificand.isPositive) std::cout << "-";
	for (indexType i = 0; i < numSigDigits; i++) {
		if (mSignificand.msb-i == mRepMSB) std::cout << "(";
		std::cout << mSignificand.digits[mSignificand.msb-i];
		if (i == 0 && i+1 < numSigDigits) std::cout << ".";
		if (mSignificand.msb-i == mRepLSB) std::cout << ")";
	}
	
	// Print the exponent
	indexType numExpDigits = mExponent.msb - mExponent.lsb + 1;
	std::cout << "e";
	if (!mExponent.isPositive) std::cout << "-";
	for (indexType i = 0; i < numExpDigits; i++)
		std::cout << mExponent.digits[mExponent.msb-i];
}
void newBigNum::print(indexType firstNDigits) const {
	if (mSignificand.size == 0 || mSignificand.lsb < 0 || mSignificand.msb < mSignificand.lsb || mExponent.size == 0 || mExponent.lsb < 0 || mExponent.msb < mExponent.lsb) {
		std::cerr << "No digits found\n";
		return;
	}

	// If the exponent is too large, print in scientific notation
	indexType numExpDigits = mExponent.msb - mExponent.lsb + 1;
	if (numExpDigits > std::numeric_limits<sizeType>::digits10) {
		printFloat();
		return;
	}

	// Get the value of the exponent
	indexType exp = 0;
	for (indexType i = 0; i < numExpDigits && mExponent.lsb+i < mExponent.size; i++)
		exp += static_cast<indexType>(std::pow(10, i)) * mExponent.digits[mExponent.lsb+i];
	exp *= mExponent.isPositive ? 1 : -1;

	// Get the number of digits to print
	indexType numSigDigits = mSignificand.msb - mSignificand.lsb + 1;
	indexType numExtraDigits = (exp < 0) ? -exp : exp-(numSigDigits-1);
	if (numExtraDigits < 0) numExtraDigits = 0;
	indexType numDigits = numSigDigits + numExtraDigits;
	if (firstNDigits > 0 && firstNDigits < numDigits) {
		std::cout << "Displaying the first " << firstNDigits << " digits:\n";
		numDigits = firstNDigits;
	}

	// Set the indexes for the radix and repetend position
	sizeType repSize = 0;
	indexType repStart = -1, repEnd = -1;
	indexType radix = (exp > 0) ? exp : 0;
	if (mRepLSB >= 0) {
		repSize = mRepMSB - mRepLSB + 1;
		repStart = mSignificand.msb - mRepMSB + numExtraDigits;
		if (repStart <= radix) repStart += (radix-repStart+1);	// repetend must be right of the radix
		repEnd = repStart + repSize - 1;
		// shift the repetend indexes to the left as much as possible
		indexType i = mRepMSB+1, j = mRepLSB;
		while (repStart-1 > radix) {
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
		if (repEnd >= numDigits && firstNDigits <= 0) {
			numExtraDigits += (repEnd-numDigits+1);
			numDigits += (repEnd-numDigits+1);
		}
	}

	// Print the number
	if (!mSignificand.isPositive) std::cout << "-";
	for (indexType i = 0, s = mSignificand.msb, r = mRepMSB; i < numDigits; i++) {
		if (i == repStart) std::cout << "(";

		// Print the i'th digit of the number
		if (exp >= 0) {
			if (i < numSigDigits) std::cout << mSignificand.digits[s--];// print digits of the significand
			else if (repSize > 0) {										// print extra digits from scaling
				std::cout << mSignificand.digits[r--];
				if (r < mRepLSB) r = mRepMSB;
			} else std::cout << "0";
		} else {
			if (i < numExtraDigits) std::cout << "0";					// print extra digits from scaling
			else if (s >= mRepLSB) std::cout << mSignificand.digits[s--];// print digits of the significand
		}

		if (i == repEnd) { std::cout << ")"; break; }
		if (i == radix && i+1 < numDigits) std::cout << ".";
	}
}
void newBigNum::shift(indexType shiftAmt) {
	// add shiftAmt to mExponent.digits

	if (shiftAmt == 0) return;
	bool shiftLeft = (shiftAmt >= 0);
	if (shiftAmt < 0) shiftAmt = -shiftAmt;
	indexType numShiftDigits = static_cast<indexType>(std::log10(shiftAmt)) + 1;
	if (mExponent.size < numShiftDigits) resizeExponent(numShiftDigits + 1);

	if (mExponent.isPositive == shiftLeft) {
		// add shiftAmt to exponent
		digitType carry = 0, i = mExponent.lsb;
		while (i < mExponent.size && (carry || shiftAmt)) {
			mExponent.digits[i] += (shiftAmt % 10) + carry;
			if (mExponent.digits[i] > 9) {
				mExponent.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
			shiftAmt /= 10;
			i++;
		}
		if (carry) {
			resizeExponent(static_cast<indexType>(mExponent.size) + 1);
			mExponent.digits[i] = carry;
			mExponent.msb = i;
		} else if (i-1 > mExponent.msb) mExponent.msb = i-1;
	} else {
		// subtract shiftAmt from exponent
		indexType numExpDigits = mExponent.msb - mExponent.lsb + 1;
		indexType numDigits = numExpDigits > numShiftDigits ? numExpDigits : numShiftDigits;
		for (indexType i = mExponent.lsb, j = 0; j < numDigits; i++, j++)
			mExponent.digits[i] = 9 - mExponent.digits[i];
		digitType carry = 0;
		for (indexType i = mExponent.lsb, j = 0; j < numDigits; i++, j++) {
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
			indexType i = mExponent.lsb;
			while (carry && i < mExponent.size) {
				mExponent.digits[i] += carry;
				if (mExponent.digits[i] > 9) mExponent.digits[i] -= 10;
				else carry = 0;
				i++;
			}
			if (i-1 > mExponent.msb) mExponent.msb = i-1;
		} else {		// result of subtraction is positive
			for (indexType i = mExponent.lsb, j = 0; j < numDigits; i++, j++)
				mExponent.digits[i] = 9 - mExponent.digits[i];
			while (mExponent.digits[mExponent.msb] == 0 && mExponent.msb > mExponent.lsb) mExponent.msb--;
		}
	}
	if (mExponent.msb == mExponent.lsb && mExponent.digits[mExponent.lsb] == 0) mExponent.isPositive = true;	// check for -zero
}
void newBigNum::shift(numArrayType e) {

}
newBigNum::numArrayType newBigNum::addNumArray(const numArrayType &lhs, const numArrayType &rhs, indexType lhsShift, indexType rhsShift) {
	
	// Add "extra" digits to the left of the smaller number's msb
	indexType shiftDiff = lhsShift - rhsShift;	// positive if lhs is smaller, negative if rhs is
	indexType shiftAmt = (shiftDiff < 0) ? -shiftDiff : shiftDiff;
	indexType lhsNumDigits = (lhs.msb - lhs.lsb + 1) + (shiftDiff < 0 ? shiftAmt : 0);
	indexType rhsNumDigits = (rhs.msb - rhs.lsb + 1) + (shiftDiff > 0 ? shiftAmt : 0);

	// Adjust the start index of the number with the larger shift to account for the difference
	indexType lhsStart = lhs.lsb - (shiftDiff > 0 ? shiftAmt : 0);
	indexType rhsStart = rhs.lsb - (shiftDiff < 0 ? shiftAmt : 0);

	// Allocate space for the sum
	indexType numDigits = (lhsNumDigits > rhsNumDigits ? lhsNumDigits : rhsNumDigits) + 1;
	if (numDigits > mMaxDigits) throw std::overflow_error("Overflow in addNumArray");
	sizeType sumNumDigits = static_cast<sizeType>(numDigits);
	sizeType sumSize = (sumNumDigits > mDefaultSize) ? sumNumDigits : mDefaultSize;
	numArrayType sum(sumSize);
	
	// Calculate the sum
	sum.isPositive = lhs.isPositive;
	if (lhs.isPositive == rhs.isPositive) {
		// Add the digits
		digitType carry = 0;
		for (indexType i = 0, l = lhsStart, r = rhsStart; i < sumNumDigits && i < sumSize; i++, l++, r++) {
			digitType left = (l < lhs.lsb || l > lhs.msb) ? 0 : lhs.digits[l];
			digitType right = (r < rhs.lsb || r > rhs.msb) ? 0 : rhs.digits[r];
			sum.digits[i] = left + right + carry;
			if (sum.digits[i] > 9) {
				sum.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
	} else {
		// Subtract the digits
		for (indexType i = 0, l = lhsStart; i < sumNumDigits && i < sumSize; i++, l++)
			sum.digits[i] = 9 - (l < lhs.lsb || l > lhs.msb ? 0 : lhs.digits[l]);
		digitType carry = 0;
		for (indexType i = 0, r = rhsStart; i < sumNumDigits && i < sumSize; i++, r++) {
			sum.digits[i] += (r < rhs.lsb || r > rhs.msb ? 0 : rhs.digits[r]) + carry;
			if (sum.digits[i] > 9) {
				sum.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
		if (carry) {	// result of subtraction is negative, |lhs| < |rhs|
			sum.isPositive = !sum.isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			indexType i = 0;
			while (carry && i < sumNumDigits && i < sumSize) {
				sum.digits[i] += carry;
				if (sum.digits[i] > 9) sum.digits[i] -= 10;
				else carry = 0;
				i++;
			}
		} else {		// result of subtraction is positive, |lhs| >= |rhs|
			for (indexType i = 0; i < sumNumDigits && i < sumSize; i++)
				sum.digits[i] = 9 - sum.digits[i];
		}
	}

	// Set the result
	sum.lsb = 0;
	sum.msb = sumNumDigits-1;
	while (sum.digits[sum.msb] == 0 && sum.msb > sum.lsb) sum.msb--;

	return sum;
}
newBigNum::numArrayType newBigNum::subtractNumArray(const numArrayType &lhs, const numArrayType &rhs, indexType lhsShift, indexType rhsShift) {

	// Add additional digits to the larger number to account for the overall shift
	indexType shiftDiff = lhsShift - rhsShift;
	indexType shiftAmt = (shiftDiff < 0) ? -shiftDiff : shiftDiff;
	indexType lhsNumDigits = (lhs.msb - lhs.lsb + 1) + (shiftDiff > 0 ? shiftAmt : 0);
	indexType rhsNumDigits = (rhs.msb - rhs.lsb + 1) + (shiftDiff < 0 ? shiftAmt : 0);

	// Allocate space for the difference
	indexType numDigits = (lhsNumDigits > rhsNumDigits) ? lhsNumDigits : rhsNumDigits;
	if (numDigits > mMaxDigits) throw std::overflow_error("Overflow in addNumArray");
	sizeType diffNumDigits = static_cast<sizeType>(numDigits);
	sizeType diffSize = (diffNumDigits > mDefaultSize) ? diffNumDigits : mDefaultSize;
	numArrayType diff(diffSize);
	diff.isPositive = lhs.isPositive;

	// Calculate the difference
	indexType lhsStart = lhs.lsb - (shiftDiff > 0 ? shiftAmt : 0);
	indexType rhsStart = rhs.lsb - (shiftDiff < 0 ? shiftAmt : 0);
	if (lhs.isPositive == rhs.isPositive) {
		// Subtract the digits
		for (indexType i = 0, l = lhsStart; i < diffNumDigits && i < diffSize; i++, l++)
			diff.digits[i] = 9 - (l < lhs.lsb || l > lhs.msb ? 0 : lhs.digits[l]);
		digitType carry = 0;
		for (indexType i = 0, r = rhsStart; i < diffNumDigits && i < diffSize; i++, r++) {
			diff.digits[i] += (r < rhs.lsb || r > rhs.msb ? 0 : rhs.digits[r]) + carry;
			if (diff.digits[i] > 9) {
				diff.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
		if (carry) {	// result of subtraction is negative, |lhs| < |rhs|
			diff.isPositive = !diff.isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			indexType i = 0;
			while (carry && i < diffNumDigits && i < diffSize) {
				diff.digits[i] += carry;
				if (diff.digits[i] > 9) diff.digits[i] -= 10;
				else carry = 0;
				i++;
			}
		} else {		// result of subtraction is positive, |lhs| >= |rhs|
			for (indexType i = 0; i < diffNumDigits && i < diffSize; i++)
				diff.digits[i] = 9 - diff.digits[i];
		}
	} else {
		// Add the digits
		digitType carry = 0;
		for (indexType i = 0, l = lhsStart, r = rhsStart; i < diffNumDigits && i < diffSize; i++, l++, r++) {
			digitType left = (l < lhs.lsb || l > lhs.msb) ? 0 : lhs.digits[l];
			digitType right = (r < rhs.lsb || r > rhs.msb) ? 0 : rhs.digits[r];
			diff.digits[i] = left + right + carry;
			if (diff.digits[i] > 9) {
				diff.digits[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
	}

	// Set the result
	diff.lsb = 0;
	diff.msb = diffNumDigits-1;
	while (diff.digits[diff.msb] == 0 && diff.msb > diff.lsb) diff.msb--;

	return diff;
}
//void newBigNum::absValSubtractNumArray(const numArrayType &lhs, indexType lStart, const numArrayType &rhs, indexType rStart, numArrayType &diff, sizeType diffNumDigits) {
//	// lhs and rhs have the same sign
//
//	if (!diff.digits) return;
//	
//	// Subtract the digits (method of complements)
//	for (indexType i = 0, l = lStart; i < diffNumDigits && i < diff.size; i++, l++)
//		diff.digits[i] = 9 - (l < lhs.lsb || l > lhs.msb ? 0 : lhs.digits[l]);
//	digitType carry = 0;
//	for (indexType i = 0, r = rStart; i < diffNumDigits && i < diff.size; i++, r++) {
//		diff.digits[i] += (r < rhs.lsb || r > rhs.msb ? 0 : rhs.digits[r]) + carry;
//		if (diff.digits[i] > 9) {
//			diff.digits[i] -= 10;
//			carry = 1;
//		} else carry = 0;
//	}
//	if (carry) {	// result of subtraction is negative, |lhs| < |rhs|
//		diff.isPositive = !diff.isPositive;
//		// add back the carry to the least significand digit (end-around carry) to get the correct difference
//		indexType i = 0;
//		while (carry && i < diffNumDigits && i < diff.size) {
//			diff.digits[i] += carry;
//			if (diff.digits[i] > 9) diff.digits[i] -= 10;
//			else carry = 0;
//			i++;
//		}
//	} else {		// result of subtraction is positive, |lhs| >= |rhs|
//		for (indexType i = 0; i < diffNumDigits && i < diff.size; i++)
//			diff.digits[i] = 9 - diff.digits[i];
//	}
//
//	// Set the result
//	diff.lsb = 0;
//	diff.msb = diffNumDigits-1;
//	while (diff.digits[diff.msb] == 0 && diff.msb > diff.lsb) diff.msb--;
//}
void newBigNum::resize(numArrayType &number, indexType size) {
	if (size < 0 || size <= number.size) return;
	if (size > mMaxDigits) throw std::overflow_error("Overflow in resize");
	sizeType newSize = static_cast<sizeType>(size);
	digitType *digits = new digitType[newSize]();
	for (indexType i = 0; i < number.size && i < newSize; i++)
		digits[i] = number.digits[i];
	if (number.digits) delete[] number.digits;
	number.digits = digits;
	number.size = newSize;
}
void newBigNum::resizeExponent(indexType size) {
	if (size < 0 || size <= mExponent.size) return;
	if (size > mMaxDigits) throw std::overflow_error("Overflow in resizeExponent");
	sizeType newSize = static_cast<sizeType>(size);
	digitType *exp = new digitType[newSize]();
	for (indexType i = 0; i < mExponent.size && i < newSize; i++)
		exp[i] = mExponent.digits[i];
	delete[] mExponent.digits;
	mExponent.digits = exp;
	mExponent.size = newSize;
}
void newBigNum::resizeSignificand(indexType size) {
	if (size < 0 || size <= mSignificand.size) return;
	if (size > mMaxDigits) throw std::overflow_error("Overflow in resizeSignificand");
	sizeType newSize = static_cast<sizeType>(size);
	digitType *sig = new digitType[newSize]();
	for (indexType i = 0; i < mSignificand.size && i < newSize; i++)
		sig[i] = mSignificand.digits[i];
	delete[] mSignificand.digits;
	mSignificand.digits = sig;
	mSignificand.size = newSize;
}
void newBigNum::clear() {
	for (indexType i = 0; i < mSignificand.size; i++) mSignificand.digits[i] = 0;
	for (indexType j = 0; j < mExponent.size; j++) mExponent.digits[j] = 0;
	mSignificand.lsb = 0;
	mSignificand.msb = 0;
	mExponent.lsb = 0;
	mExponent.msb = 0;
	mSignificand.isPositive = true;
	mExponent.isPositive = true;
	mRepLSB = -1;
	mRepMSB = -1;
}

/* ------------------------------------------------------ */
/*                      Comparison                        */
/* -------------------------------------------------------*/

bool newBigNum::operator==(const newBigNum &rhs) const {
	const newBigNum &lhs = *this;

	indexType l = lhs.mSignificand.msb;
	indexType r = rhs.mSignificand.msb;

	if (l >= lhs.mSignificand.size || r >= rhs.mSignificand.size || l < lhs.mSignificand.lsb || r < rhs.mSignificand.lsb || lhs.mSignificand.lsb < 0 || rhs.mSignificand.lsb < 0) {
		std::cerr << "Error reading operands in operator==";
		return false;
	}

	// Check if either significand is zero and compare their signs
	bool lIsZero = true;
	for (; l >= lhs.mSignificand.lsb; l--)
		if (lhs.mSignificand.digits[l] != 0) {
			lIsZero = false;
			break;
		}
	bool rIsZero = true;
	for (; r >= rhs.mSignificand.lsb; r--)
		if (rhs.mSignificand.digits[r] != 0) {
			rIsZero = false;
			break;
		}
	if (lIsZero && rIsZero) return true;
	else if (lIsZero != rIsZero) return false;
	else if (lhs.mSignificand.isPositive != rhs.mSignificand.isPositive) return false;

	// Check all significand digits
	for (l = lhs.mSignificand.msb, r = rhs.mSignificand.msb; l >= lhs.mSignificand.lsb && r >= rhs.mSignificand.lsb; l--, r--)
		if (lhs.mSignificand.digits[l] != rhs.mSignificand.digits[r]) return false;
	indexType lRep = lhs.mRepMSB;
	indexType rRep = rhs.mRepMSB;
	while (l >= lhs.mSignificand.lsb) {	// check the remaining lhs digits
		digitType rDigit = 0;
		if (rRep >= 0) {
			rDigit = rhs.mSignificand.digits[rRep];
			if (--rRep < rhs.mRepLSB) rRep = rhs.mRepMSB;
		}
		if (lhs.mSignificand.digits[l] != rDigit) return false;
		l--;
	}
	while (r >= rhs.mSignificand.lsb) {	// check the remaining rhs digits
		digitType lDigit = 0;
		if (lRep >= 0) {
			lDigit = lhs.mSignificand.digits[lRep];
			if (--lRep < lhs.mRepLSB) lRep = lhs.mRepMSB;
		}
		if (rhs.mSignificand.digits[r] != lDigit) return false;
		r--;
	}

	if (lhs.mExponent.size == 0 || lhs.mExponent.lsb < 0 || lhs.mExponent.msb < lhs.mExponent.lsb || rhs.mExponent.size == 0 || rhs.mExponent.lsb < 0 || rhs.mExponent.msb < rhs.mExponent.lsb) {
		std::cerr << "Error reading operands in operator==";
		return false;
	}

	// Check if either exponent is zero and compare their signs
	bool lExpIsZero = true;
	for (l = lhs.mExponent.msb; l >= lhs.mExponent.lsb; l--)
		if (lhs.mExponent.digits[l] != 0) {
			lExpIsZero = false;
			break;
		}
	if (l < lhs.mExponent.lsb) l++;
	bool rExpIsZero = true;
	for (r = rhs.mExponent.msb; r >= rhs.mExponent.lsb; r--)
		if (rhs.mExponent.digits[r] != 0) {
			rExpIsZero = false;
			break;
		}
	if (r < rhs.mExponent.lsb) r++;

	if (lExpIsZero != rExpIsZero) return false;
	else if (!lExpIsZero) {	// both exponents are non-zero
		if (lhs.mExponent.isPositive != rhs.mExponent.isPositive) return false;	// different signed exponents
		else if (l - lhs.mExponent.lsb != r - rhs.mExponent.lsb) return false;	// different length exponents
		else {	// Check all exponent digits
			for (; l >= lhs.mExponent.lsb && r >= rhs.mExponent.lsb; l--, r--)
				if (lhs.mExponent.digits[l] != rhs.mExponent.digits[r]) return false;
		}
	}
	
	// Check if either significand has a repetend
	if (lhs.mRepLSB >= 0 || rhs.mRepLSB >= 0) {
		if (lhs.mRepLSB >= 0 && rhs.mRepLSB < 0) {			// only lhs has a repetend
			for (lRep = lhs.mRepMSB; lRep >= lhs.mRepLSB; lRep--)
				if (lhs.mSignificand.digits[lRep] != 0) return false;
		} else if (rhs.mRepLSB >= 0 && lhs.mRepLSB < 0) {	// only rhs has a repetend
			for (rRep = rhs.mRepMSB; rRep >= rhs.mRepLSB; rRep--)
				if (rhs.mSignificand.digits[rRep] != 0) return false;
		} else {											// both have a repetend
			indexType lRepSize = lhs.mRepMSB - lhs.mRepLSB + 1;
			indexType rRepSize = rhs.mRepMSB - rhs.mRepLSB + 1;

			// line up the repetends if the significands are different lengths
			indexType lRepStart = lhs.mRepMSB, rRepStart = rhs.mRepMSB;
			indexType lSigSize = lhs.mSignificand.msb - lhs.mSignificand.lsb + 1;
			indexType rSigSize = rhs.mSignificand.msb - rhs.mSignificand.lsb + 1;
			if (lSigSize > rSigSize) rRepStart -= (lSigSize-rSigSize) % rRepSize;
			else if (rSigSize > lSigSize) lRepStart -= (rSigSize-lSigSize) % lRepSize;

			// extend each number's repetend, the most extra digits you need to compare is the lcm(lRepSize, rRepSize)
			indexType numExtraDigits = std::lcm(lRepSize, rRepSize);
			lRep = lRepStart;
			rRep = rRepStart;
			for (indexType i = 0; i < numExtraDigits; i++) {
				if (lhs.mSignificand.digits[lRep] != rhs.mSignificand.digits[rRep]) return false;
				if (--lRep < lhs.mRepLSB) lRep = lhs.mRepMSB;
				if (--rRep < rhs.mRepLSB) rRep = rhs.mRepMSB;
			}
		}
	}

	return true;
}
bool newBigNum::operator<(const newBigNum &rhs) const {
	const newBigNum &lhs = *this;

	indexType l = lhs.mSignificand.msb;
	indexType r = rhs.mSignificand.msb;

	if (l >= lhs.mSignificand.size || r >= rhs.mSignificand.size || l < lhs.mSignificand.lsb || r < rhs.mSignificand.lsb || lhs.mSignificand.lsb < 0 || rhs.mSignificand.lsb < 0) {
		std::cerr << "Error reading operands in operator<";
		return false;
	}

	// Check if either significand is zero and compare their signs
	bool lIsZero = true;
	for (; l >= lhs.mSignificand.lsb; l--)
		if (lhs.mSignificand.digits[l] != 0) {
			lIsZero = false;
			break;
		}
	bool rIsZero = true;
	for (; r >= rhs.mSignificand.lsb; r--)
		if (rhs.mSignificand.digits[r] != 0) {
			rIsZero = false;
			break;
		}
	if (lIsZero && rIsZero) return false;
	else if (lIsZero) return rhs.mSignificand.isPositive;	// (lhs is 0) true if rhs is positive
	else if (rIsZero) return !lhs.mSignificand.isPositive;	// (rhs is 0) true if lhs is negative
	else if (lhs.mSignificand.isPositive != rhs.mSignificand.isPositive) return rhs.mSignificand.isPositive;	// (signs don't match) true if rhs is positive
	bool sigSign = lhs.mSignificand.isPositive;

	// Compare exponents
	if (lhs.mExponent.size == 0 || lhs.mExponent.lsb < 0 || lhs.mExponent.msb < lhs.mExponent.lsb || rhs.mExponent.size == 0 || rhs.mExponent.lsb < 0 || rhs.mExponent.msb < rhs.mExponent.lsb) {
		std::cerr << "Error reading operands in operator<";
		return false;
	}
	bool lExpIsZero = true;
	for (l = lhs.mExponent.msb; l >= lhs.mExponent.lsb; l--)	// ignore leading zeros and check if lhs exp is zero
		if (lhs.mExponent.digits[l] != 0) {
			lExpIsZero = false;
			break;
		}
	if (l < lhs.mExponent.lsb) l++;
	bool rExpIsZero = true;
	for (r = rhs.mExponent.msb; r >= rhs.mExponent.lsb; r--)	// ignore leading zeros and check if rhs exp is zero
		if (rhs.mExponent.digits[r] != 0) {
			rExpIsZero = false;
			break;
		}
	if (r < rhs.mExponent.lsb) r++;
	if (lExpIsZero != rExpIsZero) {
		// one exponent is zero
		if (lExpIsZero) return (sigSign == rhs.mExponent.isPositive);	// if the lhs has an exp of 0, lhs < rhs when the sigs and rhs exp are positive (or both are negative)
		else return (sigSign != lhs.mExponent.isPositive);			// if the rhs has an exp of 0, lhs < rhs when the sigs are positive and the lhs exp is negative (or vice versa)
	} else if (!lExpIsZero) {
		// neither exponent is zero
		if (lhs.mExponent.isPositive != rhs.mExponent.isPositive) {
			// exponents have different signs
			return (sigSign == rhs.mExponent.isPositive);				// if both sigs are positive, lhs < rhs when the rhs exp is positive (and vice versa)
		}
		bool expSign = lhs.mExponent.isPositive;
		if (l-lhs.mExponent.lsb != r-rhs.mExponent.lsb) {
			// exponents have different sizes
			if (l-lhs.mExponent.lsb < r-rhs.mExponent.lsb) return (sigSign == expSign);	// if the lhs exp is shorter, lhs < rhs when sigs and exps are both positive (or both negative)
			else return (sigSign != expSign);					// if the rhs exp is shorter, lhs < rhs when sigs are positive and exps are negative (or vice versa)
		} else {
			// exponents are the same size and sign and neither are zero, check all digits
			for (; l >= lhs.mExponent.lsb && r >= rhs.mExponent.lsb; l--, r--) {
				if (lhs.mExponent.digits[l] < rhs.mExponent.digits[r]) return (sigSign == expSign);
				else if (rhs.mExponent.digits[r] < lhs.mExponent.digits[l]) return (sigSign != expSign);
			}
		}
	}

	// Compare significands
	for (l = lhs.mSignificand.msb, r = rhs.mSignificand.msb; l >= lhs.mSignificand.lsb && r >= rhs.mSignificand.lsb; l--, r--) {
		if (lhs.mSignificand.digits[l] < rhs.mSignificand.digits[r]) return sigSign;
		else if (rhs.mSignificand.digits[r] < lhs.mSignificand.digits[l]) return !sigSign;
	}
	indexType lRep = lhs.mRepMSB;
	indexType rRep = rhs.mRepMSB;
	while (l >= lhs.mSignificand.lsb) {	// check the remaining lhs digits
		digitType rDigit = 0;
		if (rRep >= 0) {
			rDigit = rhs.mSignificand.digits[rRep];
			if (--rRep < rhs.mRepLSB) rRep = rhs.mRepMSB;
		}
		if (lhs.mSignificand.digits[l] < rDigit) return sigSign;
		else if (rDigit < lhs.mSignificand.digits[l]) return !sigSign;
		l--;
	}
	while (r >= rhs.mSignificand.lsb) {	// check the remaining rhs digits
		digitType lDigit = 0;
		if (lRep >= 0) {
			lDigit = lhs.mSignificand.digits[lRep];
			if (--lRep < lhs.mRepLSB) lRep = lhs.mRepMSB;
		}
		if (lDigit < rhs.mSignificand.digits[r]) return sigSign;
		else if (rhs.mSignificand.digits[r] < lDigit) return !sigSign;
		r--;
	}

	// Check if either significand has a repetend
	if (lhs.mRepLSB >= 0 || rhs.mRepLSB >= 0) {
		if (lhs.mRepLSB >= 0 && rhs.mRepLSB < 0) {			// only lhs has a repetend
			for (lRep = lhs.mRepMSB; lRep >= lhs.mRepLSB; lRep--)
				if (lhs.mSignificand.digits[lRep] != 0) return sigSign;
		} else if (rhs.mRepLSB >= 0 && lhs.mRepLSB < 0) {	// only rhs has a repetend
			for (rRep = rhs.mRepMSB; rRep >= rhs.mRepLSB; rRep--)
				if (rhs.mSignificand.digits[rRep] != 0) return !sigSign;
		} else {											// both have a repetend
			indexType lRepSize = lhs.mRepMSB - lhs.mRepLSB + 1;
			indexType rRepSize = rhs.mRepMSB - rhs.mRepLSB + 1;

			// line up the repetends if the significands are different lengths
			indexType lRepStart = lhs.mRepMSB, rRepStart = rhs.mRepMSB;
			indexType lSigSize = lhs.mSignificand.msb - lhs.mSignificand.lsb + 1;
			indexType rSigSize = rhs.mSignificand.msb - rhs.mSignificand.lsb + 1;
			if (lSigSize > rSigSize) rRepStart -= (lSigSize-rSigSize) % rRepSize;
			else if (rSigSize > lSigSize) lRepStart -= (rSigSize-lSigSize) % lRepSize;

			// extend each number's repetend, the most extra digits you need to compare is the lcm(lRepSize, rRepSize)
			indexType numExtraDigits = std::lcm(lRepSize, rRepSize);
			lRep = lRepStart;
			rRep = rRepStart;
			for (indexType i = 0; i < numExtraDigits; i++) {
				if (lhs.mSignificand.digits[lRep] < rhs.mSignificand.digits[rRep]) return sigSign;
				else if (rhs.mSignificand.digits[rRep] < lhs.mSignificand.digits[lRep]) return !sigSign;
				if (--lRep < lhs.mRepLSB) lRep = lhs.mRepMSB;
				if (--rRep < rhs.mRepLSB) rRep = rhs.mRepMSB;
			}
		}
	}
	return false;	// numbers are equal
}
bool newBigNum::operator!=(const newBigNum &rhs) const {
	return !(*this == rhs);
}
bool newBigNum::operator>(const newBigNum &rhs) const {
	return (rhs < *this);
}
bool newBigNum::operator>=(const newBigNum &rhs) const {
	return !(*this < rhs);
}
bool newBigNum::operator<=(const newBigNum &rhs) const {
	return !(*this > rhs);
}

/* ------------------------------------------------------ */
/*                      Arithmetic                        */
/* -------------------------------------------------------*/

// Addition/Subtraction
newBigNum::indexType newBigNum::expDifference(const newBigNum &lhs, const newBigNum &rhs) {
	// Returns the difference: (LHSexp - RHSexp) as an indexType variable
	// If the absolute value of the difference is greater than mMaxDigits, an arbitrary value greater than mMaxDigits is returned instead

	indexType LHSexpNumDigits = lhs.mExponent.msb - lhs.mExponent.lsb + 1;
	indexType RHSexpNumDigits = rhs.mExponent.msb - rhs.mExponent.lsb + 1;
	const indexType maxNumDigitsReturnType = static_cast<indexType>(std::log10(std::numeric_limits<indexType>::max())) + 1;

	if (LHSexpNumDigits < maxNumDigitsReturnType && RHSexpNumDigits < maxNumDigitsReturnType) {
		// both exponents are small enough to be directly converted to indexType
		indexType lExp = 0, rExp = 0;
		for (indexType i = lhs.mExponent.lsb, e = 1; i <= lhs.mExponent.msb; i++, e *= 10)
			lExp += e * lhs.mExponent.digits[i];
		if (!lhs.mExponent.isPositive) lExp = -lExp;
		for (indexType i = rhs.mExponent.lsb, e = 1; i <= rhs.mExponent.msb; i++, e *= 10)
			rExp += e * rhs.mExponent.digits[i];
		if (!rhs.mExponent.isPositive) rExp = -rExp;
		return (lExp - rExp);
	}

	// exponents are too large to convert to indexType, perform subtraction digit by digit

	sizeType diffSize = (LHSexpNumDigits > RHSexpNumDigits ? LHSexpNumDigits : RHSexpNumDigits) + 1;
	digitType *diffArray = new digitType[diffSize]();	// the absolute value of the difference between exponents
	bool diffIsPositive = true;							// the sign of the difference

	// Calculate the difference
	if (lhs.mExponent.isPositive == rhs.mExponent.isPositive) {
		// calculate LHSexp - RHSexp
		for (indexType l = lhs.mExponent.lsb, i = 0; i < diffSize-1; l++, i++)
			diffArray[i] = 9 - (l <= lhs.mExponent.msb ? lhs.mExponent.digits[l] : 0);
		digitType carry = 0;
		for (indexType r = rhs.mExponent.lsb, i = 0; i < diffSize-1; r++, i++) {
			diffArray[i] += (r <= rhs.mExponent.msb ? rhs.mExponent.digits[r] : 0) + carry;
			if (diffArray[i] > 9) {
				diffArray[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
		if (carry) {	// result of subtraction is negative, |LHSexp| < |RHSexp|
			diffIsPositive = !lhs.mExponent.isPositive;
			// add back the carry to the least significand digit (end-around carry) to get the correct difference
			indexType i = 0;
			while (carry && i < diffSize) {
				diffArray[i] += carry;
				if (diffArray[i] > 9) diffArray[i] -= 10;
				else carry = 0;
				i++;
			}
		} else {		// result of subtraction is positive, |LHSexp| >= |RHSexp|
			diffIsPositive = lhs.mExponent.isPositive;
			for (indexType i = 0; i < diffSize-1; i++)
				diffArray[i] = 9 - diffArray[i];
		}
	} else {
		// calculate LHSexp + RHSexp
		digitType carry = 0;
		for (indexType l = lhs.mExponent.lsb, r = rhs.mExponent.lsb, i = 0; i < diffSize; i++, l++, r++) {
			diffArray[i] = (l <= lhs.mExponent.msb ? lhs.mExponent.digits[l] : 0) + (r <= rhs.mExponent.msb ? rhs.mExponent.digits[r] : 0) + carry;
			if (diffArray[i] > 9) {
				diffArray[i] -= 10;
				carry = 1;
			} else carry = 0;
		}
		diffIsPositive = lhs.mExponent.isPositive;
	}

	// Convert the digit by digit difference to an indexType variable
	indexType diff = 0;
	indexType msb = diffSize-1;
	while (diffArray[msb] == 0 && msb > 0) msb--;
	if (msb+1 > static_cast<indexType>(std::log10(mMaxDigits))+1) {	// exponent difference has too many digits
		diff = mMaxDigits+1;	// an arbitrary value greater than mMaxDigits
	} else {
		for (indexType i = 0; i < diffSize; i++)
			diff += static_cast<indexType>(std::pow(10, i)) * diffArray[i];
		if (!diffIsPositive) diff = -diff;
	}
	delete[] diffArray;
	return diff;
}

void newBigNum::absValueAdd(const newBigNum &rhs) {

	newBigNum &lhs = *this;

	// Find the difference between the LHS and RHS exponents
	indexType lExp = 0, rExp = 0, expDiff = 0;
	if (lhs.mExponent.toInteger(lExp) && rhs.mExponent.toInteger(rExp)) {
		expDiff = lExp - rExp;
	} else {
		numArrayType numArrayDiff = subtractNumArray(lhs.mExponent, rhs.mExponent);
		if (!numArrayDiff.toInteger(expDiff)) throw std::overflow_error("Overflow in absValAdd");
	}
	indexType shiftAmt = (expDiff < 0) ? -expDiff : expDiff;
	if (shiftAmt > mMaxDigits) throw std::overflow_error("Overflow in absValAdd");
	//indexType LHSexpNumDigits = lhs.mExponent.msb - lhs.mExponent.lsb + 1;
	//indexType RHSexpNumDigits = rhs.mExponent.msb - rhs.mExponent.lsb + 1;
	//const indexType maxNumDigits = static_cast<indexType>(std::log10(std::numeric_limits<indexType>::max())) + 1;
	//indexType expDiff = 0;
	//if (LHSexpNumDigits < maxNumDigits && RHSexpNumDigits < maxNumDigits) {
	//	// both exponents are small enough to be directly converted to indexType
	//	indexType lExp = 0, rExp = 0;
	//	for (indexType i = lhs.mExponent.lsb, e = 1; i <= lhs.mExponent.msb; i++, e *= 10)
	//		lExp += e * lhs.mExponent.digits[i];
	//	if (!lhs.mExponent.isPositive) lExp = -lExp;
	//	for (indexType i = rhs.mExponent.lsb, e = 1; i <= rhs.mExponent.msb; i++, e *= 10)
	//		rExp += e * rhs.mExponent.digits[i];
	//	if (!rhs.mExponent.isPositive) rExp = -rExp;
	//	expDiff = lExp - rExp;
	//} else {
	//	// calculate the difference digit by digit
	//	numArrayType eDiff = subtractNumArray(lhs.mExponent, rhs.mExponent);
	//	if (eDiff.msb - eDiff.lsb + 1 > static_cast<indexType>(std::log10(mMaxDigits))+1)
	//		expDiff = mMaxDigits+1;	// exponent difference is too big, return arbitrary value > mMaxDigits
	//	else {
	//		for (indexType i = eDiff.lsb; i <= eDiff.msb && i < eDiff.size; i++)
	//			expDiff += static_cast<indexType>(std::pow(10, i)) * eDiff.digits[i];
	//		if (!eDiff.isPositive) expDiff = -expDiff;
	//	}
	//}
	//indexType shiftAmt = (expDiff < 0) ? -expDiff : expDiff;
	//if (shiftAmt > mMaxDigits) throw std::overflow_error("Overflow in absValAdd");

	// Set the exponent of the sum
	if (expDiff < 0) mExponent = rhs.mExponent;

	// Set the significand of the sum
	indexType lhsShift = (expDiff > 0) ? shiftAmt : 0;
	indexType rhsShift = (expDiff < 0) ? shiftAmt : 0;
	indexType lhsNumSigDigits = (lhs.mSignificand.msb - lhs.mSignificand.lsb + 1) + lhsShift;
	indexType rhsNumSigDigits = (rhs.mSignificand.msb - rhs.mSignificand.lsb + 1) + rhsShift;
	mSignificand = addNumArray(lhs.mSignificand, rhs.mSignificand, lhsShift, rhsShift);

	// Check if the addition resulted in an additional digit
	indexType sumMSB = (lhsNumSigDigits > rhsNumSigDigits ? lhsNumSigDigits : rhsNumSigDigits) - 1;
	indexType numExtraDigits = mSignificand.msb - sumMSB;
	if (numExtraDigits) shift(numExtraDigits);
	if (mSignificand.msb == mSignificand.lsb && mSignificand.digits[mSignificand.lsb] == 0) mSignificand.isPositive = true;	// check for -0
}
//void newBigNum::absValueAdd(const newBigNum &rhs) {
//
//	newBigNum &lhs = *this;
//
//	// Find the difference between the LHS and RHS exponents
//	indexType expDiff = expDifference(lhs, rhs);
//	if (expDiff > mMaxDigits) throw std::overflow_error("Overflow in absValAdd");
//	indexType shiftAmt = (expDiff < 0) ? -expDiff : expDiff;
//	bool lhsExpIsLarger = (lhs.mExponent.isPositive == rhs.mExponent.isPositive) ? (expDiff > 0) : lhs.mExponent.isPositive;
//
//	// Determine the number of digits in the sum
//	indexType LHSnumDigits = (lhs.mSignificand.msb - lhs.mSignificand.lsb + 1) + (!lhsExpIsLarger ? shiftAmt : 0);
//	indexType RHSnumDigits = (rhs.mSignificand.msb - rhs.mSignificand.lsb + 1) + (lhsExpIsLarger ? shiftAmt : 0);
//	indexType numDigits = (LHSnumDigits > RHSnumDigits ? LHSnumDigits : RHSnumDigits) + 1;
//	if (numDigits > mMaxDigits) throw std::overflow_error("Overflow in absValAdd");
//	sizeType sumNumDigits = static_cast<sizeType>(numDigits);
//
//	// Set the exponent of the sum
//	if (!lhsExpIsLarger) {
//		if (rhs.mExponent.msb >= mExponent.size) {
//			mExponent.size = rhs.mExponent.size;
//			delete[] mExponent.digits;
//			mExponent.digits = new digitType[mExponent.size]();
//		}
//		for (indexType i = rhs.mExponent.lsb; i <= rhs.mExponent.msb; i++)
//			mExponent.digits[i] = rhs.mExponent.digits[i];
//		mExponent.lsb = rhs.mExponent.lsb;
//		mExponent.msb = rhs.mExponent.msb;
//		mExponent.isPositive = rhs.mExponent.isPositive;
//	}
//
//	// Allocate space
//	sizeType sumSize = sumNumDigits > mSignificand.size ? sumNumDigits : mSignificand.size;
//	digitType *sum = new digitType[sumSize]();
//
//	// Get indexes of both numbers
//	indexType lStart = LHSnumDigits > RHSnumDigits ? lhs.mSignificand.lsb : lhs.mSignificand.lsb-(RHSnumDigits-LHSnumDigits);
//	indexType rStart = LHSnumDigits > RHSnumDigits ? rhs.mSignificand.lsb-(LHSnumDigits-RHSnumDigits) : rhs.mSignificand.lsb;
//
//	// Add digits
//	digitType carry = 0, i = 0;
//	for (indexType l = lStart, r = rStart; i < sumNumDigits-1 && i < sumSize-1; i++, l++, r++) {
//		digitType left = (l < lhs.mSignificand.lsb || l > lhs.mSignificand.msb) ? 0 : lhs.mSignificand.digits[l];
//		digitType right = (r < rhs.mSignificand.lsb || r > rhs.mSignificand.msb) ? 0 : rhs.mSignificand.digits[r];
//		sum[i] = left + right + carry;
//		if (sum[i] > 9) {
//			sum[i] -= 10;
//			carry = 1;
//		} else carry = 0;
//	}
//	sum[i] = carry;
//	if (carry) lhs.shift(1);	// An additional digit was added to the sum
//	else i--;
//
//	// Set the significand of the sum
//	delete[] mSignificand.digits;
//	mSignificand.digits = sum;
//	mSignificand.size = sumSize;
//	mSignificand.lsb = 0;
//	mSignificand.msb = i;
//}
void newBigNum::absValueSubtract(const newBigNum &rhs) {

	newBigNum &lhs = *this;

	// Find the difference between the LHS and RHS exponents
	indexType expDiff = expDifference(lhs, rhs);
	if (expDiff > mMaxDigits) throw std::overflow_error("Overflow in absValSubtract");
	indexType shiftAmt = (expDiff < 0) ? -expDiff : expDiff;
	bool lhsExpIsLarger = (lhs.mExponent.isPositive == rhs.mExponent.isPositive) ? (expDiff >= 0) : lhs.mExponent.isPositive;

	// Determine the number of digits in the difference
	indexType LHSnumDigits = (lhs.mSignificand.msb - lhs.mSignificand.lsb + 1) + (!lhsExpIsLarger ? shiftAmt : 0);
	indexType RHSnumDigits = (rhs.mSignificand.msb - rhs.mSignificand.lsb + 1) + (lhsExpIsLarger ? shiftAmt : 0);
	indexType numDigits = (LHSnumDigits > RHSnumDigits) ? LHSnumDigits : RHSnumDigits;
	if (numDigits > mMaxDigits) throw std::overflow_error("Overflow in absValSubtract");
	sizeType diffNumDigits = static_cast<sizeType>(numDigits);

	// Set the exponent of the difference
	if (!lhsExpIsLarger) {
		if (rhs.mExponent.msb >= mExponent.size) {
			mExponent.size = rhs.mExponent.size;
			delete[] mExponent.digits;
			mExponent.digits = new digitType[mExponent.size]();
		}
		for (indexType i = rhs.mExponent.lsb; i <= rhs.mExponent.msb; i++)
			mExponent.digits[i] = rhs.mExponent.digits[i];
		mExponent.lsb = rhs.mExponent.lsb;
		mExponent.msb = rhs.mExponent.msb;
		mExponent.isPositive = rhs.mExponent.isPositive;
	}

	// Allocate space
	sizeType diffSize = diffNumDigits > mSignificand.size ? diffNumDigits : mSignificand.size;
	digitType *diff = new digitType[diffSize]();

	// Get indexes of both numbers
	indexType lStart = LHSnumDigits > RHSnumDigits ? lhs.mSignificand.lsb : lhs.mSignificand.lsb-(RHSnumDigits-LHSnumDigits);
	indexType rStart = LHSnumDigits > RHSnumDigits ? rhs.mSignificand.lsb-(LHSnumDigits-RHSnumDigits) : rhs.mSignificand.lsb;

	// Subtract digits (using method of complements)
	for (indexType i = 0, l = lStart; i < diffNumDigits && i < diffSize; i++, l++)
		diff[i] = 9 - (l < lhs.mSignificand.lsb || l > lhs.mSignificand.msb ? 0 : lhs.mSignificand.digits[l]);
	digitType carry = 0;
	for (indexType i = 0, r = rStart; i < diffNumDigits && i < diffSize; i++, r++) {
		diff[i] += (r < rhs.mSignificand.lsb || r > rhs.mSignificand.msb ? 0 : rhs.mSignificand.digits[r]) + carry;
		if (diff[i] > 9) {
			diff[i] -= 10;
			carry = 1;
		} else carry = 0;
	}
	if (carry) {	// result of subtraction is negative
		mSignificand.isPositive = !mSignificand.isPositive;
		// add back the carry to the least significand digit (end-around carry) to get the correct difference
		indexType i = 0;
		while (carry && i < diffNumDigits && i < diffSize) {
			diff[i] += carry;
			if (diff[i] > 9) diff[i] -= 10;
			else carry = 0;
			i++;
		}
	} else {		// result of subtraction is positive
		for (indexType i = 0; i < diffNumDigits && i < diffSize; i++)
			diff[i] = 9 - diff[i];
	}

	// Set the significand of the difference
	delete[] mSignificand.digits;
	mSignificand.digits = diff;
	mSignificand.size = diffSize;
	mSignificand.lsb = 0;
	mSignificand.msb = diffNumDigits-1;
	indexType lostDigits = 0;
	while (mSignificand.digits[mSignificand.msb] == 0 && mSignificand.msb > mSignificand.lsb) {
		mSignificand.msb--;
		lostDigits++;
	}
	if (lostDigits) shift(-lostDigits);	// difference starts with 0's
	if (mSignificand.msb == mSignificand.lsb && mSignificand.digits[mSignificand.lsb] == 0) mSignificand.isPositive = true;	// check for -0
}
newBigNum& newBigNum::operator+=(const newBigNum &rhs) {
	if (mSignificand.isPositive == rhs.mSignificand.isPositive)
		absValueAdd(rhs);
	else
		absValueSubtract(rhs);
	return *this;
}
newBigNum& newBigNum::operator-=(const newBigNum &rhs) {
	if (mSignificand.isPositive == rhs.mSignificand.isPositive)
		absValueSubtract(rhs);
	else
		absValueAdd(rhs);
	return *this;
}
newBigNum operator+(newBigNum lhs, const newBigNum &rhs) {
	lhs += rhs;
	return lhs;
}
newBigNum operator-(newBigNum lhs, const newBigNum &rhs) {
	lhs -= rhs;
	return lhs;
}

// Multiplication
void newBigNum::longMultiply(const newBigNum &rhs) {

	newBigNum &lhs = *this;

	// Find the difference between the LHS and RHS exponents
	indexType expDiff = expDifference(lhs, rhs);
	if (expDiff > mMaxDigits) throw std::overflow_error("Overflow in absValAdd");
	indexType shiftAmt = (expDiff < 0) ? -expDiff : expDiff;
	bool lhsExpIsLarger = (lhs.mExponent.isPositive == rhs.mExponent.isPositive) ? (expDiff > 0) : lhs.mExponent.isPositive;

	// Determine the number of digits in the sum
	indexType LHSnumDigits = (lhs.mSignificand.msb - lhs.mSignificand.lsb + 1) + (!lhsExpIsLarger ? shiftAmt : 0);
	indexType RHSnumDigits = (rhs.mSignificand.msb - rhs.mSignificand.lsb + 1) + (lhsExpIsLarger ? shiftAmt : 0);
	indexType numDigits = (LHSnumDigits > RHSnumDigits ? LHSnumDigits : RHSnumDigits) + 1;
	if (numDigits > mMaxDigits) throw std::overflow_error("Overflow in absValAdd");
	sizeType sumNumDigits = static_cast<sizeType>(numDigits);

	// Set the exponent of the sum
	if (!lhsExpIsLarger) {
		if (rhs.mExponent.msb >= mExponent.size) {
			mExponent.size = rhs.mExponent.size;
			delete[] mExponent.digits;
			mExponent.digits = new digitType[mExponent.size]();
		}
		for (indexType i = rhs.mExponent.lsb; i <= rhs.mExponent.msb; i++)
			mExponent.digits[i] = rhs.mExponent.digits[i];
		mExponent.lsb = rhs.mExponent.lsb;
		mExponent.msb = rhs.mExponent.msb;
		mExponent.isPositive = rhs.mExponent.isPositive;
	}

	// Allocate space
	sizeType sumSize = sumNumDigits > mSignificand.size ? sumNumDigits : mSignificand.size;
	digitType *sum = new digitType[sumSize]();

	// Get indexes of both numbers
	indexType lStart = LHSnumDigits > RHSnumDigits ? lhs.mSignificand.lsb : lhs.mSignificand.lsb-(RHSnumDigits-LHSnumDigits);
	indexType rStart = LHSnumDigits > RHSnumDigits ? rhs.mSignificand.lsb-(LHSnumDigits-RHSnumDigits) : rhs.mSignificand.lsb;
}