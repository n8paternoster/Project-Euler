#pragma once

typedef int16_t digitType;
typedef uint32_t sizeType;

class bigNum {

public:
	// Members
	digitType *mDigits;			// numbers are stored little endian, i.e. the least significant digit is index 0
	sizeType mSize;
	sizeType mNumDigits;
	sizeType mIntegerStart;		// index of the least significant integer digit, i.e. the one's place
	sizeType mRepetendStart;	// index of the least significant repetend digit of the first repetend period
	sizeType mRepetendSize;		// length of the repetend
	bool mIsNegative;
	bool mIsInteger;
	bool mIsZero;

	const static sizeType defaultPrecision;

	// Utility
	bool updateState();				// updates mNumDigits, mIsZero, and mIsInteger
	bool removeLeadingZeros();
	bool removeTrailingZeros();
	bool shiftLeft(sizeType);
	bool shiftRight(sizeType);
	bool padMSB(sizeType);
	bool padLSB(sizeType, bool = false);
	bool resize(sizeType);
	digitType& operator[](sizeType);
	friend uint64_t toUInt64(const bigNum&);

	// Multiplication
	bool splitAt(sizeType, bigNum&, bigNum&) const;
	bool setKaratsubaDecimalPoint(const bigNum&, const bigNum&);
	bigNum integerKaratsuba(bigNum, bigNum) const;
	bigNum absValMultiply(bigNum, bigNum) const;

	// Division
	bigNum newtonRaphsonDivide(const bigNum&, const bigNum&) const;
	bigNum extractBigNum(sizeType, sizeType) const;
	bigNum multiplyByDigit(digitType) const;
	bigNum divideByDigit(digitType, bigNum&) const;
	bigNum integerLongDivision(const bigNum&, const bigNum&, bigNum&) const;
	bool precisionLongDivision(const bigNum&, const bigNum&, bigNum&, sizeType) const;
	bool precisionDivideByDigit(digitType, const bigNum&, bigNum&) const;

	// Addition/Subtraction
	bigNum absValAdd(bigNum&, bigNum&) const;
	bigNum absValSubtract(bigNum&, bigNum&) const;

//public:

	// Constructors
	bigNum(sizeType length = 50);
	bigNum(std::string);
	bigNum(const bigNum&);

	// Destructor
	~bigNum();

	// Copy assignment operator
	bigNum& operator=(const bigNum&);

	// Getters
	sizeType numDigits() const;
	sizeType repetendPeriod() const;
	bool isNegative() const;
	bool isInteger() const;
	digitType operator[](sizeType) const;

	// Utility
	void setNegative();
	void setPositive();
	void print(sizeType firstNDigits = 0) const;
	void printAll() const;
	bigNum round(const bigNum&, sizeType) const;


	// Comparison operators
	bool operator==(const bigNum&) const;
	bool operator!=(const bigNum&) const;
	bool operator>(const bigNum&) const;
	bool operator<(const bigNum&) const;
	bool operator>=(const bigNum&) const;
	bool operator<=(const bigNum&) const;

	// Arithmetic
	bigNum operator+(const bigNum&) const;
	bigNum operator-(const bigNum&) const;
	bigNum operator*(const bigNum&) const;
	bigNum operator/(const bigNum&) const;
	bigNum operator%(const bigNum&) const;
	bigNum operator^(uint32_t) const;
	friend bigNum factorial(const bigNum&);		// fix for negative nums
	friend bool divideWithPrecision(const bigNum& dividend, const bigNum& divisor, bigNum& quotient, sizeType p);
};


//bigNum round(const bigNum&, sizeType);		// make friend function?


