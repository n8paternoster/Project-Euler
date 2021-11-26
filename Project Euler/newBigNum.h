#pragma once

class newBigNum {

private:
	using digitType = uint16_t;
	using sizeType = uint32_t;
	using indexType = int64_t;
	enum struct DIVMODE { QUOTIENT, EUCLIDQUOTIENT, EUCLIDREMAINDER };
	struct numArrayType {
		digitType*	digits;
		sizeType	size;
		indexType	lsb;
		indexType	msb;
		bool		isPositive;
		bool toInteger(indexType&) const;
		numArrayType();
		numArrayType(sizeType);
		numArrayType(const numArrayType&);
		numArrayType& operator=(const numArrayType&);
		numArrayType(numArrayType&&) noexcept;
		numArrayType& operator=(numArrayType&&) noexcept;
		~numArrayType();
	};
	numArrayType	mSignificand;
	numArrayType	mExponent;
	indexType		mRepLSB;
	indexType		mRepMSB;
	//digitType*	mSignificand;
	//digitType*	mExponent;
	//sizeType		mSigSize;
	//indexType		mSigLSB;
	//indexType		mSigMSB;
	//bool			mSigIsPositive;
	//sizeType		mExpSize;
	//indexType		mExpLSB;
	//indexType		mExpMSB;
	//bool			mExpIsPositive;

	static constexpr indexType mMaxDigits = std::numeric_limits<sizeType>::max();	// max # of digits for sig and exp
	static const sizeType mDefaultSize = 50;
	static inline sizeType mDivisionPrecision = 50;

	// Utility
	void resize(numArrayType&, indexType);
	void resizeExponent(indexType);		// allocate memory for the exponent
	void resizeSignificand(indexType);	// allocate memory for the significand
	void shift(indexType);				// add or subtract a static integer value from the exponent
	void shift(numArrayType);			// add or subtract a digit array from the exponent

	// Addition/Subtraction
	static numArrayType addNumArray(const numArrayType&, const numArrayType&, indexType = 0, indexType = 0);
	static numArrayType subtractNumArray(const numArrayType&, const numArrayType&, indexType = 0, indexType = 0);
	static indexType expDifference(const newBigNum&, const newBigNum&); // returns the difference between exponents as indexType
	void absValueAdd(const newBigNum&);
	void absValueSubtract(const newBigNum&);

	// Multiplication
	std::pair<newBigNum, newBigNum> splitAt(sizeType) const;
	newBigNum karatsuba(const newBigNum&, const newBigNum&) const;
	void longMultiply(const newBigNum&);

	// Division
	void multiplyByDigit(digitType);
	void divideByDigit(digitType);
	newBigNum longDivision(newBigNum, newBigNum, DIVMODE, sizeType = mDivisionPrecision);

public:

	// Special member functions
	newBigNum();											// zero-init
	explicit newBigNum(sizeType sigSize, sizeType expSize);	// zero-init, reserve space
	newBigNum(std::string);									// construct from string
	template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool> = true>
	newBigNum(Integer);										// construct from integer type
	template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool> = true>
	newBigNum(Floating);									// construct from floating-point type
	//newBigNum(const newBigNum&);							// copy c-tor
	//newBigNum(newBigNum&&) noexcept;						// move c-tor
	template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool> = true>
	newBigNum& operator=(Integer);							// assign from integer type
	template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool> = true>
	newBigNum& operator=(Floating);							// assign from floating-point type
	//newBigNum& operator=(const newBigNum&);					// copy assignment
	//newBigNum& operator=(newBigNum&&) noexcept;				// move assignment
	//~newBigNum();											// d-tor

	// Utility
	digitType& operator[](indexType i);						// access digit of significand
	const digitType& operator[](indexType i) const;	
	void print(indexType firstNDigits = 0) const;
	void printFloat() const;
	void clear();											// sets the number to 0, does not change size or reallocate
	static void setPrecision(sizeType p) {
		if (p <= 0) return;
		mDivisionPrecision = p;
	}

	// Comparison
	bool operator==(const newBigNum&) const;
	bool operator!=(const newBigNum&) const;
	bool operator>(const newBigNum&) const;
	bool operator<(const newBigNum&) const;
	bool operator>=(const newBigNum&) const;
	bool operator<=(const newBigNum&) const;

	// Arithmetic
	newBigNum& operator+=(const newBigNum&);
	newBigNum& operator-=(const newBigNum&);
	newBigNum& operator*=(const newBigNum&);
	newBigNum& operator/=(const newBigNum&);
	newBigNum& operator%=(const newBigNum&);
	newBigNum& operator^=(const newBigNum&);
	newBigNum& operator++();	// prefix
	newBigNum& operator--();
	newBigNum operator++(int);	// postfix
	newBigNum operator--(int);

};

// Construct from integer type
template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool>>
newBigNum::newBigNum(Integer num):
	mRepLSB(-1), mRepMSB(-1)
{
	bool isPos = (num >= 0);
	if (num < 0) num = -num;
	
	// Set the significand
	sizeType numSigDigits = (num == 0) ? 1 : static_cast<sizeType>(std::log10(num)) + 1;
	sizeType sigSize = (numSigDigits > mDefaultSize) ? numSigDigits : mDefaultSize;
	mSignificand = numArrayType(sigSize);
	indexType i = 0;
	do {
		mSignificand.digits[i++] = num % 10;
		num /= 10;
	} while (num != 0 && i < sigSize);
	mSignificand.lsb = 0;
	mSignificand.msb = i-1;
	mSignificand.isPositive = isPos;

	// Set the exponent
	indexType exp = numSigDigits-1;
	sizeType numExpDigits = (exp == 0) ? 1 : static_cast<sizeType>(std::log10(exp)) + 1;
	sizeType expSize = (numExpDigits > mDefaultSize) ? numExpDigits : mDefaultSize;
	mExponent = numArrayType(expSize);
	i = 0;
	do {
		mExponent.digits[i++] = exp % 10;
		exp /= 10;
	} while (exp != 0 && i < expSize);
	mExponent.lsb = 0;
	mExponent.msb = i-1;
	mExponent.isPositive = true;	// integers cannot have a negative exponent
}

// Assign from integer type
template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool>>
newBigNum& newBigNum::operator=(Integer num) {
	mSignificand.isPositive = (num >= 0);
	mExponent.isPositive = true;
	if (num < 0) num = -num;
	
	// Set the significand
	sizeType numSigDigits = (num == 0) ? 1 : static_cast<sizeType>(std::log10(num)) + 1;
	sizeType sigSize = (numSigDigits > mDefaultSize) ? numSigDigits : mDefaultSize;
	delete[] mSignificand.digits;
	mSignificand.digits = new digitType[sigSize]();
	mSignificand.size = sigSize;
	indexType i = 0;
	do {
		mSignificand.digits[i++] = num % 10;
		num /= 10;
	} while (num != 0 && i < sigSize);
	mSignificand.lsb = 0;
	mSignificand.msb = i-1;

	// Set the exponent
	indexType exp = numDigits-1;
	sizeType numExpDigits = (exp == 0) ? 1 : static_cast<sizeType>(std::log10(exp)) + 1;
	sizeType expSize = (numExpDigits > mDefaultSize) ? numExpDigits : mDefaultSize;
	delete[] mExponent.digits;
	mExponent.digits = new digitType[expSize]();
	mExponent.size = expSize;
	i = 0;
	do {
		mExponent.digits[i++] = exp % 10;
		exp /= 10;
	} while (exp != 0 && i < expSize);
	mExponent.lsb = 0;
	mExponent.msb = i-1;

	mRepLSB = -1;
	mRepMSB = -1;
	return *this;
}

// Construct from floating-point type
template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool>>
newBigNum::newBigNum(Floating num):
	mRepLSB(-1), mRepMSB(-1)
{
	// converts a floating point number into decimal scientific notation using snprintf

	if (!std::isfinite(num)) throw std::invalid_argument("Error, NaN");

	// Read num into a char buffer
	const char format[] = "%+#.*Le";	// scientific notation format, "+d.ddde+dd" with specified precision
	constexpr int maxExp = std::numeric_limits<Floating>::max_exponent10;
	sizeType maxNumSigDigits = std::numeric_limits<Floating>::digits10;
	if (maxNumSigDigits == 0) maxNumSigDigits = 1;
	sizeType maxNumExpDigits = (maxExp == 0) ? 1 : static_cast<sizeType>(std::log10(maxExp)) + 1;
	size_t buffSize =
		1 +					// significand sign, '+' or '-'
		maxNumSigDigits +	// max number of significand digits
		1 +					// radix '.'
		1 +					// exponent separator, 'e'
		1 +					// exponent sign, '+' or '-'
		maxNumExpDigits +	// max number of exponent digits
		1;					// null terminator, '/0'
	char *buffer = new char[buffSize];
	int n = snprintf(buffer, buffSize, format, maxNumSigDigits-1, num);	 // precision argument specifies # of digits *after* the decimal point
	if (n < 0 || n >= buffSize) throw std::invalid_argument("Error reading floating point number");

	indexType i = 0;	// index for buffer
	indexType j = 0;	// index for mSignificand.digits and mExponent.digits

	// --------------- Parse the significand ------------------- //

	bool isPos = (buffer[i++] == '+');

	// Count the # of significand digits
	sizeType numSigDigits = 0;
	j = i;
	while (j < n && buffer[j] != 'e') {
		if (buffer[j] >= '0' && buffer[j] <= '9') numSigDigits++;
		j++;
	}
	j--;
	while (j > i && numSigDigits > 0 && buffer[j--] == '0') numSigDigits--;	// ignore trailing zeros
	if (numSigDigits == 0) numSigDigits = 1;

	// Read the significand digits
	sizeType sigSize = (numSigDigits > mDefaultSize) ? numSigDigits : mDefaultSize;
	mSignificand = numArrayType(sigSize);
	for (j = 0; i < n && j < numSigDigits; i++) {
		char c = buffer[i];
		if (c == 'e') break;
		if (c >= '0' && c <= '9') mSignificand.digits[numSigDigits-1-(j++)] = c - '0';
	}
	mSignificand.msb = numSigDigits-1;
	mSignificand.lsb = numSigDigits-j;
	while (mSignificand.digits[mSignificand.msb] == 0 && mSignificand.msb > mSignificand.lsb) mSignificand.msb--;	// ignore leading zeros
	mSignificand.isPositive = isPos;

	// --------------- Parse the exponent ------------------- //

	while (i < n && buffer[i] != 'e') i++;	// skip the rest of the significand (trailing zeros)
	if (i < n && buffer[i] == 'e') i++;		// skip the 'e' character
	else throw std::invalid_argument("Error reading floating point number");	// sanity check
	isPos = (buffer[i++] == '+');

	// Count the # of exponent digits
	sizeType numExpDigits = 0;
	j = i;
	while (j < n && buffer[j] >= '0' && buffer[j++] <= '9') numExpDigits++;
	if (numExpDigits == 0) numExpDigits = 1;

	// Read the exponent
	sizeType expSize = (numExpDigits > mDefaultSize) ? numExpDigits : mDefaultSize;
	mExponent = numArrayType(expSize);
	for (j = 0; i < n && j < numExpDigits; i++) {
		char c = buffer[i];
		if (c >= '0' && c <= '9') mExponent.digits[numExpDigits-1-(j++)] = c - '0';
		else break;
	}
	mExponent.msb = numExpDigits-1;
	mExponent.lsb = numExpDigits-j;
	while (mExponent.digits[mExponent.msb] == 0 && mExponent.msb > mExponent.lsb) mExponent.msb--; // ignore leading zeros
	mExponent.isPositive = isPos;

	// ----------------------------------------------------------- //

	// Check if the number is zero
	if (mSignificand.lsb == mSignificand.msb && mSignificand.digits[mSignificand.lsb] == 0) {
		mSignificand.isPositive = true;
		mExponent.isPositive = true;
	}

	delete[] buffer;
}

// Assign from floating-point type
template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool>>
newBigNum& newBigNum::operator=(Floating num) {

	// converts a floating point argument into decimal scientific notation using snprintf

	if (!std::isfinite(num)) throw std::invalid_argument("Error, NaN");

	// Read num into a char buffer
	const char format[] = "%+#.*Le";	// scientific notation format, "+d.ddde+dd" with specified precision
	constexpr int maxExp = std::numeric_limits<Floating>::max_exponent10;
	sizeType maxNumSigDigits = std::numeric_limits<Floating>::digits10;
	if (maxNumSigDigits == 0) maxNumSigDigits = 1;
	sizeType maxNumExpDigits = (maxExp == 0) ? 1 : static_cast<sizeType>(std::log10(maxExp)) + 1;
	size_t buffSize =
		1 +					// significand sign, '+' or '-'
		maxNumSigDigits +	// max number of significand digits
		1 +					// radix '.'
		1 +					// exponent separator, 'e'
		1 +					// exponent sign, '+' or '-'
		maxNumExpDigits +	// max number of exponent digits
		1;					// null terminator, '/0'
	char *buffer = new char[buffSize];
	int n = snprintf(buffer, buffSize, format, maxNumSigDigits-1, num);	 // precision argument specifies # of digits *after* the decimal point
	if (n < 0 || n >= buffSize) throw std::runtime_error("Error reading floating point number");

	indexType i = 0;	// index for buffer
	indexType j = 0;	// index for mSignificand.digits and mExponent.digits

	// --------------- Parse the significand ------------------- //

	bool isPos = (buffer[i++] == '+');

	// Count the # of significand digits
	sizeType numSigDigits = 0;
	j = i;
	while (j < n && buffer[j] != 'e') {
		if (buffer[j] >= '0' && buffer[j] <= '9') numSigDigits++;
		j++;
	}
	j--;
	while (j > i && numSigDigits > 0 && buffer[j--] == '0') numSigDigits--;	// ignore trailing zeros
	if (numSigDigits == 0) numSigDigits = 1;

	// Read the significand
	sizeType sigSize = (numSigDigits > mDefaultSize) ? numSigDigits : mDefaultSize;
	mSignificand.size = sigSize;
	delete[] mSignificand.digits;
	mSignificand.digits = new digitType[sigSize]();
	for (j = 0; i < n && j < numSigDigits; i++) {
		char c = buffer[i];
		if (c == 'e') break;
		if (c >= '0' && c <= '9') mSignificand.digits[numSigDigits-1-(j++)] = c - '0';
	}
	mSignificand.msb = numSigDigits-1;
	mSignificand.lsb = numSigDigits-j;
	while (mSignificand.digits[mSignificand.msb] == 0 && mSignificand.msb > mSignificand.lsb) mSignificand.msb--;	// ignore leading zeros
	mSignificand.isPositive = isPos;

	// --------------- Parse the exponent ------------------- //

	while (i < n && buffer[i] != 'e') i++;	// skip the rest of the significand (trailing zeros)
	if (i < n && buffer[i] == 'e') i++;		// skip the 'e' character
	else throw std::invalid_argument("Error reading floating point number");	// sanity check
	isPos = (buffer[i++] == '+');

	// Count the # of exponent digits
	sizeType numExpDigits = 0;
	j = i;
	while (j < n && buffer[j] >= '0' && buffer[j++] <= '9') numExpDigits++;
	if (numExpDigits == 0) numExpDigits = 1;

	// Read the exponent
	sizeType expSize = (numExpDigits > mDefaultSize) ? numExpDigits : mDefaultSize;
	mExponent.size = expSize;
	delete[] mExponent.digits;
	mExponent.digits = new digitType[expSize]();
	for (j = 0; i < n && j < numExpDigits; i++) {
		char c = buffer[i];
		if (c >= '0' && c <= '9') mExponent.digits[numExpDigits-1-(j++)] = c - '0';
		else break;
	}
	mExponent.msb = numExpDigits-1;
	mExponent.lsb = numExpDigits-j;
	while (mExponent.digits[mExponent.msb] == 0 && mExponent.msb > mExponent.lsb) mExponent.msb--; // ignore leading zeros
	mExponent.isPositive = isPos;

	// ----------------------------------------------------------- //

	// Check if the number is zero
	if (mSignificand.lsb == mSignificand.msb && mSignificand.digits[mSignificand.lsb] == 0) {
		mSignificand.isPositive = true;
		mExponent.isPositive = true;
	}

	delete[] buffer;
	mRepLSB = -1;
	mRepMSB = -1;
	return *this;
}

newBigNum operator+(newBigNum lhs, const newBigNum &rhs);
newBigNum operator-(newBigNum lhs, const newBigNum &rhs);
newBigNum operator*(newBigNum lhs, const newBigNum &rhs);
newBigNum operator/(newBigNum lhs, const newBigNum &rhs);
newBigNum operator%(newBigNum lhs, const newBigNum &rhs);
newBigNum operator^(newBigNum lhs, const newBigNum &rhs);