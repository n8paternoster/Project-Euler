#pragma once

class bigNum {
	public:
	using digit_type = uint16_t;
	using size_type = uint32_t;
	using index_type = int64_t;
	enum struct div_mode { default, precision, euclid };

	struct num_type {		// represents an integer as an array of decimal digits
		digit_type*		digits;
		size_type		size;
		index_type		lsb;
		index_type		msb;
		bool			isPositive;

		num_type();						// zero-init
		explicit num_type(size_type);	// zero-init, reserve space
		num_type(const num_type&);
		num_type(num_type&&) noexcept;
		num_type& operator=(const num_type&);
		num_type& operator=(num_type&&) noexcept;
		~num_type();

		digit_type operator[](index_type) const;	// access element (const)
		digit_type& operator[](index_type);			// access element
		void clear();								// set number to 0, does not reallocate
		void resize(index_type);					// allocate additional space
		bool toInteger(index_type&) const;			// convert value to built-in type, return by ref
		std::pair<num_type, num_type> splitAt(index_type) const;
		index_type add(const num_type&, index_type = 0);
		index_type subtract(const num_type&, index_type = 0);
		void longMultiply(const num_type&);
		index_type divideDigits(div_mode, num_type&, digit_type*, index_type, index_type);
		index_type longDivide(div_mode, num_type, index_type = 0);
	};

	num_type	mSignificand;
	num_type	mExponent;
	index_type	mRepLSB;
	index_type	mRepMSB;

	static constexpr index_type mMaxDigits = std::numeric_limits<size_type>::max();	// max # of digits for num_type
	static constexpr size_type	mDefaultSize = 50;
	static constexpr size_type	mDefaultDecimalPrecision = 20;
	static inline size_type		mPrecision = 0;

	// Utility
	static bool integerDifference(const num_type&, const num_type&, index_type&);
	void shift(index_type);

	// Arithmetic
	num_type karatsuba(const num_type&, const num_type&) const;

public:

	// Constructors & assignment operators
	bigNum();												// zero-init
	explicit bigNum(size_type sigSize, size_type expSize);	// zero-init, reserve space
	bigNum(std::string);									// construct from string
	template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool> = true>
	bigNum(Integer);										// construct from integer type
	template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool> = true>
	bigNum(Floating);										// construct from floating-point type
	template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool> = true>
	bigNum& operator=(Integer);								// assign from integer type
	template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool> = true>
	bigNum& operator=(Floating);							// assign from floating-point type

	// Utility
	void print(index_type firstNDigits = 0) const;
	void printFloat() const;
	index_type numDigits() const { return mSignificand.msb - mSignificand.lsb + 1; }
	digit_type operator[](index_type i) { return mSignificand[i]; }
	static void setPrecision(size_type precision) {
		if (precision <= 0) throw std::invalid_argument("Invalid precision");
		mPrecision = precision;
	}
	static void unsetPrecision() { mPrecision = 0; }


	// Comparison
	bool operator==(const bigNum&) const;
	bool operator<(const bigNum&) const;
	bool operator!=(const bigNum&) const;
	bool operator>(const bigNum&) const;
	bool operator<=(const bigNum&) const;
	bool operator>=(const bigNum&) const;

	// Compound assignment operators
	bigNum& operator+=(const bigNum&);
	bigNum& operator-=(const bigNum&);
	bigNum& operator*=(const bigNum&);
	bigNum& operator/=(const bigNum&);
	bigNum& operator%=(const bigNum&);
};

// Binary arithmetic operators
bigNum operator+(bigNum, const bigNum&);
bigNum operator-(bigNum, const bigNum&);
bigNum operator*(bigNum, const bigNum&);
bigNum operator/(bigNum, const bigNum&);
bigNum operator%(bigNum, const bigNum&);

// Ostream overload for printing
std::ostream& operator<<(std::ostream&, const bigNum&);

// Construct from integer type
template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool>>
bigNum::bigNum(Integer num):
	mSignificand(), mExponent(), mRepLSB(-1), mRepMSB(-1)
{
	// Set the significand
	mSignificand.isPositive = (num >= 0);
	if (num < 0) num = -num;
	size_type numSigDigits = (num == 0) ? 1 : static_cast<size_type>(std::log10(num)) + 1;
	if (numSigDigits > mSignificand.size) mSignificand.resize(numSigDigits);
	index_type i = 0;
	do {
		mSignificand.digits[i++] = num % 10;
		num /= 10;
	} while (num != 0 && i < mSignificand.size);
	mSignificand.lsb = 0;
	mSignificand.msb = i-1;

	// Exponent is 0
}

// Assign from integer type
template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool>>
bigNum& bigNum::operator=(Integer num) {
	mSignificand.isPositive = (num >= 0);
	if (num < 0) num = -num;

	// Set the significand
	size_type numSigDigits = (num == 0) ? 1 : static_cast<size_type>(std::log10(num)) + 1;
	size_type sigSize = (numSigDigits > mSignificand.size) ? numSigDigits : mSignificand.size;
	delete[] mSignificand.digits;
	mSignificand.digits = new digit_type[sigSize]();
	mSignificand.size = sigSize;
	index_type i = 0;
	do {
		mSignificand.digits[i++] = num % 10;
		num /= 10;
	} while (num != 0 && i < sigSize);
	mSignificand.lsb = 0;
	mSignificand.msb = i-1;

	// Exponent is 0
	mExponent.clear();

	mRepLSB = -1;
	mRepMSB = -1;
	return *this;
}

// Construct from floating-point type
template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool>>
bigNum::bigNum(Floating num):
	mSignificand(), mExponent(), mRepLSB(-1), mRepMSB(-1)
{
	// Convert the floating-point number into a char array using snprintf

	if (!std::isfinite(num)) throw std::invalid_argument("Error, NaN");

	// Read num into a char buffer
	const char format[] = "%+#.*Le";	// scientific notation format, "+d.ddde+dd" with specified precision
	constexpr int maxExp = std::numeric_limits<Floating>::max_exponent10;
	size_type maxNumSigDigits = std::numeric_limits<Floating>::digits10;
	if (maxNumSigDigits == 0) maxNumSigDigits = 1;
	size_type maxNumExpDigits = (maxExp == 0) ? 1 : static_cast<size_type>(std::log10(maxExp)) + 1;
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
	if (n < 0 || (size_t)n >= buffSize) throw std::invalid_argument("Error reading floating point number");

	index_type i = 0;	// index for buffer
	index_type j = 0;	// index for mSignificand and mExponent

	// --------------- Parse the significand ------------------- //

	mSignificand.isPositive = (buffer[i++] == '+');

	// Count the # of significand digits
	size_type numSigDigits = 0;
	j = i;
	while (j < n && buffer[j] != 'e') {
		if (buffer[j] >= '0' && buffer[j] <= '9') numSigDigits++;
		j++;
	}
	j--;
	while (j > i && numSigDigits > 0 && buffer[j--] == '0') numSigDigits--;	// ignore trailing zeros
	if (numSigDigits == 0) numSigDigits = 1;

	// Read the significand digits
	if (numSigDigits > mSignificand.size) mSignificand.resize(numSigDigits);
	for (j = 0; j < numSigDigits && i < n; i++) {
		char c = buffer[i];
		if (c == 'e') break;
		if (c >= '0' && c <= '9') mSignificand.digits[numSigDigits-1-(j++)] = c - '0';
	}
	mSignificand.lsb = 0;
	mSignificand.msb = numSigDigits-1;

	// Check if the number is zero
	if (mSignificand.lsb == mSignificand.msb && mSignificand.digits[mSignificand.lsb] == 0) {
		mSignificand.isPositive = true;		// no -zero representation
		delete[] buffer;
		return;
	}

	// --------------- Parse the exponent ------------------- //

	while (i < n && buffer[i] != 'e') i++;	// skip the rest of the significand (trailing zeros)
	if (i < n && buffer[i] == 'e') i++;		// skip the 'e' character
	else throw std::invalid_argument("Error reading floating point number");	// sanity check
	bool expIsPositive = (buffer[i++] == '+');

	// Count the # of exponent digits
	size_type numExpDigits = 0;
	j = i;
	while (j < n && buffer[j] >= '0' && buffer[j++] <= '9') numExpDigits++;
	if (numExpDigits == 0) numExpDigits = 1;

	// Get the value of the exponent
	index_type exp = 0;
	for (j = 0; j < numExpDigits && i+j < n; j++)
		exp += static_cast<index_type>(std::pow(10, numExpDigits-1-j)) * (buffer[i+j] - '0');
	if (!expIsPositive) exp = -exp;
	exp -= (numSigDigits-1);	// adjust the given exponent so that the significand is an integer

	// Set the exponent digits
	if (numExpDigits > mExponent.size) mExponent.resize(numExpDigits);
	mExponent.isPositive = (exp >= 0);
	if (exp < 0) exp = -exp;
	j = 0;
	do {
		mExponent.digits[j++] = exp % 10;
		exp /= 10;
	} while (exp != 0 && j < mExponent.size);
	mExponent.lsb = 0;
	mExponent.msb = j-1;

	delete[] buffer;
}

// Assign from floating-point type
template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool>>
bigNum& bigNum::operator=(Floating num) {

	// Convert the floating-point number into a char array using snprintf

	if (!std::isfinite(num)) throw std::invalid_argument("Error, NaN");

	// Read num into a char buffer
	const char format[] = "%+#.*Le";	// scientific notation format, "+d.ddde+dd" with specified precision
	constexpr int maxExp = std::numeric_limits<Floating>::max_exponent10;
	size_type maxNumSigDigits = std::numeric_limits<Floating>::digits10;
	if (maxNumSigDigits == 0) maxNumSigDigits = 1;
	size_type maxNumExpDigits = (maxExp == 0) ? 1 : static_cast<size_type>(std::log10(maxExp)) + 1;
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
	if (n < 0 || (size_t)n >= buffSize) throw std::invalid_argument("Error reading floating point number");

	index_type i = 0;	// index for buffer
	index_type j = 0;	// index for mSignificand and mExponent

	// --------------- Parse the significand ------------------- //

	mSignificand.isPositive = (buffer[i++] == '+');

	// Count the # of significand digits
	size_type numSigDigits = 0;
	j = i;
	while (j < n && buffer[j] != 'e') {
		if (buffer[j] >= '0' && buffer[j] <= '9') numSigDigits++;
		j++;
	}
	j--;
	while (j > i && numSigDigits > 0 && buffer[j--] == '0') numSigDigits--;	// ignore trailing zeros
	if (numSigDigits == 0) numSigDigits = 1;

	// Read the significand digits
	size_type sigSize = (numSigDigits > mSignificand.size) ? numSigDigits : mSignificand.size;
	delete[] mSignificand.digits;
	mSignificand.digits = new digit_type[sigSize]();
	mSignificand.size = sigSize;
	for (j = 0; j < numSigDigits && i < n; i++) {
		char c = buffer[i];
		if (c == 'e') break;
		if (c >= '0' && c <= '9') mSignificand.digits[numSigDigits-1-(j++)] = c - '0';
	}
	mSignificand.lsb = 0;
	mSignificand.msb = numSigDigits-1;

	mRepLSB = -1;
	mRepMSB = -1;

	// Check if the number is zero
	if (mSignificand.lsb == mSignificand.msb && mSignificand.digits[mSignificand.lsb] == 0) {
		mSignificand.isPositive = true;	// no -zero representation
		mExponent.clear();				// set the exponent to 0
		delete[] buffer;
		return *this;
	}

	// --------------- Parse the exponent ------------------- //

	while (i < n && buffer[i] != 'e') i++;	// skip the rest of the significand (trailing zeros)
	if (i < n && buffer[i] == 'e') i++;		// skip the 'e' character
	else throw std::invalid_argument("Error reading floating point number");	// sanity check
	bool expIsPositive = (buffer[i++] == '+');

	// Count the # of exponent digits
	size_type numExpDigits = 0;
	j = i;
	while (j < n && buffer[j] >= '0' && buffer[j++] <= '9') numExpDigits++;
	if (numExpDigits == 0) numExpDigits = 1;

	// Get the value of the exponent
	index_type exp = 0;
	for (j = 0; j < numExpDigits && i+j < n; j++)
		exp += static_cast<index_type>(std::pow(10, numExpDigits-1-j)) * (buffer[i+j] - '0');
	if (!expIsPositive) exp = -exp;
	exp -= (numSigDigits-1);	// adjust the given exponent so that the significand is an integer

	// Set the exponent digits
	size_type expSize = (numExpDigits > mExponent.size) ? numExpDigits : mExponent.size;
	delete[] mExponent.digits;
	mExponent.digits = new digit_type[expSize]();
	mExponent.size = expSize;
	mExponent.isPositive = (exp >= 0);
	if (exp < 0) exp = -exp;
	j = 0;
	do {
		mExponent.digits[j++] = exp % 10;
		exp /= 10;
	} while (exp != 0 && j < mExponent.size);
	mExponent.lsb = 0;
	mExponent.msb = j-1;

	delete[] buffer;
	return *this;
}