#include "pch.h"
#include "76-100/93.h"
#include <string>
#include <algorithm>	// std::next_permutation, std::prev_permutation, std::sort, std::find
#include <array>
#include <vector>
#include <tuple>
#include <set>

/* Brute force solution, generate every possible arithmetic expression involving 4 terms and the 4 basic operators
* 
* Iterate through all valid sets of 4 numbers (9 choose 4 = 126 sets)
* Iterate through all permutations of each set (4! = 24 permutations)
* Iterate through all arrangements of 3 operators (4*4*4 = 64 arrangements)
* Iterate through all distinct parentheses arrangements (for 4 numbers there are 5 arrangements, corresponding to the 3rd Catalan number, see arrangements below)
* Total expressions to evaluate = 126 * 24 * 64 * 5 = 967,680
* 
* Numbers are stored as floats to account for intermediate operations being non-integers
*/

constexpr int maxNumber = 9;			// starting from 1
constexpr int setSize = 4;
constexpr int numOpCodes = 4;			// 0 = addition, 1 = subtraction, 2 = multiplication, 3 = division
constexpr int numParenthesesCodes = 5;	// 0 = ((ab)c)d, 1 = (a(bc))d, 2 = a(b(cd)), 3 = a(b(cd)), 4 = (ab)(cd)
constexpr int maxSolution = (maxNumber)*(maxNumber-1)*(maxNumber-2)*(maxNumber-3);
using setArray = std::array<float, setSize>;
using opArray = std::array<int, setSize-1>;
using expressionType = std::tuple<setArray, opArray, int, int>;	// digits, operators, parentheses code, result

inline float calculate(float x, float y, int opCode, bool &valid) {
	switch (opCode) {
		case 0: return x + y;
		case 1: return x - y;
		case 2: return x * y;
		case 3:
			if ((y > 0 ? (y < 0.001f) : (y > -0.001f))) {	// divide by 0
				valid = false;
				return 0.f;
			}
			return x / y;
	}
	valid = false;
	return 0.f;
}

int evaluateExpression(const setArray &digits, int pCode, const opArray &opCode, bool &valid) {
	valid = true;
	float result = 0;
	switch (pCode) {
		case 0:		// ((a + b) + c) + d
			result = calculate(digits[0], digits[1], opCode[0], valid);
			result = calculate(result, digits[2], opCode[1], valid);
			result = calculate(result, digits[3], opCode[2], valid);
			break;
		case 1:		// (a + (b + c)) + d
			result = calculate(digits[1], digits[2], opCode[1], valid);
			result = calculate(digits[0], result, opCode[0], valid);
			result = calculate(result, digits[3], opCode[2], valid);
			break;
		case 2:		// a + ((b + c) + d)
			result = calculate(digits[1], digits[2], opCode[1], valid);
			result = calculate(result, digits[3], opCode[2], valid);
			result = calculate(digits[0], result, opCode[0], valid);
			break;
		case 3:		// a + (b + (c + d))
			result = calculate(digits[2], digits[3], opCode[2], valid);
			result = calculate(digits[1], result, opCode[1], valid);
			result = calculate(digits[0], result, opCode[0], valid);
			break;
		case 4: {	// (a + b) + (c + d) 
			result = calculate(digits[0], digits[1], opCode[0], valid);
			float temp = calculate(digits[2], digits[3], opCode[2], valid);
			result = calculate(result, temp, opCode[1], valid);
			break;
		}
		default:
			valid = false;
			break;
	}
	if (result != static_cast<int>(result)) valid = false;
	return static_cast<int>(result);
}

// Print an expression and its solution to console
void printExpression(expressionType e) {
	setArray &digits = std::get<0>(e);
	opArray &opCode = std::get<1>(e);
	char op[setSize-1] = {};
	for (size_t i = 0; i < opCode.size(); i++) {
		switch (opCode[i]) {
			case 0: op[i] = '+'; break;
			case 1: op[i] = '-'; break;
			case 2: op[i] = '*'; break;
			case 3: op[i] = '/'; break;
		}
	}
	switch (std::get<2>(e)) {
		case 0:		// ((a + b) + c) + d
			std::cout << "((" << digits[0] << " " << op[0] << " " << digits[1] << ") " << op[1] << " " << digits[2] << ") " << op[2] << " " << digits[3] << " = " << std::get<3>(e) << "\n";
			break;
		case 1:		// (a + (b + c)) + d
			std::cout << "(" << digits[0] << " " << op[0] << " (" << digits[1] << " " << op[1] << " " << digits[2] << ")) " << op[2] << " " << digits[3] << " = " << std::get<3>(e) << "\n";
			break;
		case 2:		// a + ((b + c) + d)
			std::cout << digits[0] << " " << op[0] << " ((" << digits[1] << " " << op[1] << " " << digits[2] << ") " << op[2] << " " << digits[3] << ") = " << std::get<3>(e) << "\n";
			break;
		case 3:		// a + (b + (c + d))
			std::cout << digits[0] << " " << op[0] << " (" << digits[1] << " " << op[1] << " (" << digits[2] << " " << op[2] << " " << digits[3] << ")) = " << std::get<3>(e) << "\n";
			break;
		case 4:		// (a + b) + (c + d)
			std::cout << "(" << digits[0] << " " << op[0] << " " << digits[1] << ") " << op[1] << " (" << digits[2] << " " << op[2] << " " << digits[3] << ") = " << std::get<3>(e) << "\n";
			break;
	}
}

// Print all the solutions to a set of numbers provided by the user
void solveUserInput() {

	setArray set;
	std::cout << "Enter " << setSize << " numbers:\n";
	std::string buff = "";
	for (int i = 0; i < setSize; i++) {
		std::cin >> buff;
		int intBuff = std::stoi(buff);
		set[i] = static_cast<float>(intBuff);
	}
	std::sort(set.begin(), set.end());

	opArray opCode = {};		// {+, +, +}
	auto nextOpCode = [&]() -> bool {
		for (int i = 0; i < setSize-1; i++) {
			if (++opCode[i] >= numOpCodes) {
				opCode[i] = 0;
				if (i == setSize-2) return false;
			} else break;
		}
		return true;
	};
	std::set<int> solutions;
	std::vector<expressionType> expressions;
	do { // Check all permutations of this set
		do { // Check all operator arrangements
			// Check all distinct arrangements of parentheses
			for (int pCode = 0; pCode < numParenthesesCodes; pCode++) {
				bool valid = true;
				int result = evaluateExpression(set, pCode, opCode, valid);
				if (valid && result > 0 && solutions.insert(result).second) {
					expressions.push_back({set, opCode, pCode, result});
				}
			}
		} while (nextOpCode());
	} while (std::next_permutation(set.begin(), set.end()));
	std::sort(expressions.begin(), expressions.end(), [](expressionType l, expressionType r) { return std::get<3>(l) < std::get<3>(r); });

	// Print result
	std::cout << "\n(";
	for (auto f : set) std::cout << " " << static_cast<int>(f);
	std::cout << " ) produces " << solutions.size()  << " solutions:\n\n";
	for (auto &e : expressions) printExpression(e);
}

std::string longestSolutionSet() {
	std::array<bool, maxNumber+1> setMask = {};	// generate combinations using a binary mask to specify digits
	std::fill(setMask.begin()+1, setMask.begin()+1+setSize, true);	// {1, 2, 3, 4}
	opArray opCode = {};		// {+, +, +}

	// Helper fnc to generate all permutations of operator arrangements with repetition
	auto nextOpCode = [&]() -> bool {
		for (int i = 0; i < setSize-1; i++) {
			if (++opCode[i] >= numOpCodes) {
				opCode[i] = 0;
				if (i == setSize-2) return false;
			} else break;
		}
		return true;
	};

	int longestSequence = 0;
	setArray solutionSet;
	std::vector<expressionType> solutionExpressions;
	do { // Check all sets of 4 distinct digits
		setArray set;
		for (int i = 1, d = 0; i <= maxNumber; i++) if (setMask[i]) set[d++] = static_cast<float>(i);
		std::array<bool, maxSolution+1> solutions = {};
		std::vector<expressionType> expressions;
		do { // Check all permutations of this set
			do { // Check all operator arrangements
				// Check all distinct arrangements of parentheses
				for (int pCode = 0; pCode < numParenthesesCodes; pCode++) {
					bool valid = true;
					int result = evaluateExpression(set, pCode, opCode, valid);
					if (valid && result > 0 && !solutions[result]) {
						solutions[result] = true;
						expressions.push_back({set, opCode, pCode, result});
					}
				}
			} while (nextOpCode());
		} while (std::next_permutation(set.begin(), set.end()));
		int sequenceLength = std::find(++solutions.begin(), solutions.end(), false) - solutions.begin() - 1;
		if (sequenceLength > longestSequence) {
			longestSequence = sequenceLength;
			solutionSet = set;
			solutionExpressions = expressions;
		}
	} while (std::prev_permutation(setMask.begin()+1, setMask.end()));

	std::sort(solutionExpressions.begin(), solutionExpressions.end(), [](expressionType l, expressionType r) { return std::get<3>(l) < std::get<3>(r); });

	std::string result = "";
	for (auto i : solutionSet) result += ('0' + static_cast<int>(i));

	// Print result
	std::cout << result << " produces " << longestSequence << " consecutive solutions:\n\n";
	for (int i = 0; i < longestSequence; i++) printExpression(solutionExpressions[i]);

	return result;
}