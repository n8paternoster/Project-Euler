#pragma once

class bigNumOLD {

	using digitType = short;
	using sizeType = unsigned int;
	friend std::hash<bigNumOLD>;

	enum struct ShiftDirection { LEFT, RIGHT };

	// Members
	digitType *mDigits;			// numbers are stored little endian, the least significant digit is index 0
	sizeType mSize;
	sizeType mNumDigits;
	sizeType mIntegerStart;		// index of the least significant integer digit, i.e. the one's place
	sizeType mRepetendStart;	// index of the least significant repetend digit of the first repetend period
	sizeType mRepetendSize;		// length of the repetend
	bool mIsNegative;
	bool mIsInteger;
	bool mIsZero;

	static const sizeType mDefaultPrecision = 50;
	static const sizeType mDefaultSize = 50;
	static sizeType mPrecision;

	// Utility
	bool countDigits();				// updates mNumDigits, mIsZero, and mIsInteger
	//bigNumOLD& shift(int);
	bigNumOLD& shift(sizeType, ShiftDirection);
	bool padMSB(sizeType);
	bool padLSB(sizeType);
	bool removeZeros(bool trailingZerosAreSig = false);

	// Multiplication
	std::pair<bigNumOLD, bigNumOLD> splitAt(sizeType) const;
	bigNumOLD karatsuba(const bigNumOLD&, const bigNumOLD&) const;
	void absValIntegerLongMultiplication(const bigNumOLD&);

	// Division
	bigNumOLD newtonRaphsonDivide(const bigNumOLD&, const bigNumOLD&) const;
	void multiplyByDigit(digitType);
	void divideByDigit(digitType);
	int trimToSignificand();
	bool precisionDivide(const bigNumOLD&, sizeType = mPrecision);

	// Addition/Subtraction
	void absValAdd(const bigNumOLD&);
	void absValSubtract(const bigNumOLD&);

public:

	bigNumOLD();
	explicit bigNumOLD(sizeType size, sizeType radixPos);	// zero-init, reserve space
	bigNumOLD(std::string);								// construct from string
	template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool> = true>
	bigNumOLD(Integer);									// construct from integer type
	template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool> = true>
	bigNumOLD(Floating);									// construct from floating-point type
	~bigNumOLD();
	bigNumOLD(const bigNumOLD&);
	bigNumOLD(bigNumOLD&&) noexcept;
	bigNumOLD& operator=(const bigNumOLD&);
	bigNumOLD& operator=(bigNumOLD&&) noexcept;
	template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool> = true>
	bigNumOLD& operator=(Integer);							// assign from integer type
	template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool> = true>
	bigNumOLD& operator=(Floating);						// assign from floating-point type

	// Getters
	sizeType numDigits() const;
	sizeType size() const;
	sizeType repetendPeriod() const;
	digitType& operator[](sizeType d);
	const digitType& operator[](sizeType d) const;

	// Utility
	void operator!() { mIsNegative = !mIsNegative; }
	void print(sizeType firstNDigits = 0) const;
	void printAll() const;
	void reset();
	friend bigNumOLD abs(const bigNumOLD&);		// absolute value
	friend bigNumOLD operator-(const bigNumOLD&);	// unary minus
	static void setPrecision(sizeType p) {
		if (p <= 0) return;
		mPrecision = p;
	}
	bigNumOLD round(const bigNumOLD&, sizeType) const;
	explicit operator int() const;

	// Comparison operators
	bool operator==(const bigNumOLD&) const;
	bool operator!=(const bigNumOLD&) const;
	bool operator>(const bigNumOLD&) const;
	bool operator<(const bigNumOLD&) const;
	bool operator>=(const bigNumOLD&) const;
	bool operator<=(const bigNumOLD&) const;

	// Arithmetic
	bigNumOLD& operator+=(const bigNumOLD&);
	bigNumOLD& operator-=(const bigNumOLD&);
	bigNumOLD& operator*=(const bigNumOLD&);
	bigNumOLD& operator/=(const bigNumOLD&);
	bigNumOLD& operator%=(const bigNumOLD&);
	bigNumOLD& operator^=(const bigNumOLD&);
	bigNumOLD& operator++();	// prefix
	bigNumOLD& operator--();
	bigNumOLD operator++(int);	// postfix
	bigNumOLD operator--(int);
	friend std::pair<bigNumOLD, bigNumOLD> euclideanDivide(bigNumOLD, bigNumOLD);
	friend bigNumOLD factorial(const bigNumOLD&);		// fix for negative nums

	enum struct DivisionMode { QUOTIENT, EUCLIDQUOTIENT, EUCLIDREMAINDER };

	bigNumOLD newDivide(bigNumOLD, bigNumOLD, DivisionMode, sizeType);

};

template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool>>
bigNumOLD::bigNumOLD(Integer num):
	mIntegerStart(0), mRepetendSize(0), mRepetendStart(0), mIsNegative(num < 0), mIsInteger(true)
{
	if (num < 0) num *= -1;
	sizeType numDigits = num > 0 ? (static_cast<sizeType>(std::log10(num)) + 1) : 1;
	mSize = numDigits;
	mDigits = new digitType[numDigits]();
	sizeType d = 0;
	while (num != 0 && d < mSize) {
		mDigits[d++] = num % 10;
		num /= 10;
	}
	countDigits();
}
template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>, bool>>
bigNumOLD& bigNumOLD::operator=(Integer num) {
	mRepetendSize = 0;
	mRepetendStart = 0;
	mIsInteger = true;
	mIntegerStart = 0;
	mIsNegative = num < 0;
	if (num < 0) num *= -1;

	sizeType numDigits = num > 0 ? (static_cast<sizeType>(log10(num)) + 1) : 1;
	if (mSize != numDigits) {		// check if a new size is needed
		delete[] mDigits;
		mSize = numDigits;
		mDigits = new digitType[numDigits]();
	}
	sizeType d = 0;
	while (num != 0 && d < mSize) {
		mDigits[d++] = num % 10;
		num /= 10;
	}
	while (d < mSize) mDigits[d++] = 0;
	countDigits();
	return *this;
}
template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool>>
bigNumOLD::bigNumOLD(Floating num):
	mRepetendSize(0), mRepetendStart(0), mIsNegative(std::signbit(num))
{
	if (!std::isfinite(num)) throw std::invalid_argument("Error, NaN");

	// Read num into a char buffer (snprintf)
	const char fmt[] = "%+#.*Le";	// scientific notation format, "+d.ddde+dd" (or -)
	int sigSize = std::numeric_limits<Floating>::digits10;
	if (sigSize <= 0) sigSize = 1;
	int maxE = std::numeric_limits<Floating>::max_exponent10;
	int maxExpSize = static_cast<int>(std::log10(std::numeric_limits<Floating>::max_exponent10)) + 1;
	int buffSize =
		1 +						// significand sign, '+' or '-'
		sigSize +				// max number of significand digits
		1 +						// '.'
		1 +						// exponent separator, 'e'
		1 +						// exponent sign, '+' or '-'
		maxExpSize +			// max number of exponent digits
		1;						// string terminator, '\0'
	char *buffer = new char[buffSize]();
	int n = std::snprintf(buffer, buffSize, fmt, sigSize-1, num);
	if (n < 0 || n >= buffSize) throw std::invalid_argument("Error reading floating point number");

	// Read the significand digits
	mSize = static_cast<sizeType>(sigSize);
	mDigits = new digitType[mSize]();
	mIntegerStart = mSize-1;
	int i = 0;
	for (sizeType j = 0; i < n && j < mSize; i++) {
		char c = buffer[i];
		if (c == 'e') break;
		if (c >= '0' && c <= '9') {
			mDigits[mSize-1-j] = c - '0';
			j++;
		}
	}

	// Read the exponent digits and shift the significand 
	i += 2;	// skip 'e' and exponent sign
	int numExpDigits = 0;
	for (int e = i; e <= n; e++) {
		if (buffer[e] >= '0' && buffer[e] <= '9') numExpDigits++;
		else break;
	}
	sizeType exp = 0;
	for (int e = 0; e < numExpDigits && i+e <= n; e++)
		exp += static_cast<sizeType>(pow(10, numExpDigits-1-e)*(buffer[i+e]-'0'));
	ShiftDirection dir = (buffer[i-1] == '+') ? ShiftDirection::LEFT : ShiftDirection::RIGHT;
	if (exp) shift(exp, dir);

	removeZeros();
	countDigits();
	delete[] buffer;
}
template <typename Floating, std::enable_if_t<std::is_floating_point_v<Floating>, bool>>
bigNumOLD& bigNumOLD::operator=(Floating num) {
	mRepetendSize = 0;
	mRepetendStart = 0;
	mIsNegative = std::signbit(num);
	if (!std::isfinite(num)) throw std::invalid_argument("Error, NaN");

	// Read num into a char buffer (snprintf)
	const char fmt[] = "%+#.*Le";	// scientific notation format, "+d.ddde+dd" (or -)
	int sigSize = std::numeric_limits<Floating>::digits10;
	if (sigSize <= 0) sigSize = 1;
	int maxE = std::numeric_limits<Floating>::max_exponent10;
	int maxExpSize = static_cast<int>(std::log10(std::numeric_limits<Floating>::max_exponent10)) + 1;
	int buffSize =
		1 +						// significand sign, '+' or '-'
		sigSize +				// max number of significand digits
		1 +						// '.'
		1 +						// exponent separator, 'e'
		1 +						// exponent sign, '+' or '-'
		maxExpSize +			// max number of exponent digits
		1;						// string terminator, '\0'
	char *buffer = new char[buffSize]();
	int n = std::snprintf(buffer, buffSize, fmt, sigSize-1, num);
	if (n < 0 || n >= buffSize) throw std::invalid_argument("Error reading floating point number");

	// Read the significand digits
	delete[] mDigits;
	mSize = static_cast<sizeType>(sigSize);
	mDigits = new digitType[mSize]();
	mIntegerStart = mSize-1;
	int i = 0;
	for (sizeType j = 0; i < n && j < mSize; i++) {
		char c = buffer[i];
		if (c == 'e') break;
		if (c >= '0' && c <= '9') {
			mDigits[mSize-1-j] = c - '0';
			j++;
		}
	}

	// Read the exponent digits and shift the significand 
	i += 2;	// skip 'e' and exponent sign
	int numExpDigits = 0;
	for (int e = i; e <= n; e++) {
		if (buffer[e] >= '0' && buffer[e] <= '9') numExpDigits++;
		else break;
	}
	int exp = 0;
	for (int e = 0; e < numExpDigits && i+e <= n; e++)
		exp += static_cast<int>(pow(10, numExpDigits-1-e)*(buffer[i+e]-'0'));
	if (buffer[i-1] == '-') exp = -exp;
	if (exp) shift(exp);

	removeZeros();
	countDigits();
	delete[] buffer;
	return *this;
}


bigNumOLD operator+(bigNumOLD lhs, const bigNumOLD &rhs);
bigNumOLD operator-(bigNumOLD lhs, const bigNumOLD &rhs);
bigNumOLD operator*(bigNumOLD lhs, const bigNumOLD &rhs);
bigNumOLD operator/(bigNumOLD lhs, const bigNumOLD &rhs);
bigNumOLD operator%(bigNumOLD lhs, const bigNumOLD &rhs);
bigNumOLD operator^(bigNumOLD lhs, const bigNumOLD &rhs);