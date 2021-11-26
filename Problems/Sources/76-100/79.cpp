#include "pch.h"
#include "76-100/79.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>    // std::sort

const int numDigits = 10;

/* Recursively add the right neighbors of r to n*/
bool addRightNeighbors(bool (&right)[numDigits][numDigits], int n, int r) {
    if (n == r) return false;
    // add all of r's right neighbors to n
    for (int i = 0; i < numDigits; i++) {
        if (right[r][i]) {
            right[n][i] = true;
            if (!addRightNeighbors(right, n, i)) return false;
        }
    }
    return true;
}

/* Solved this problem by hand
   This function was written as a quick, dirty code solution that will work as long as the passcode does not contain any repeated digits
   If there are multiple solutions only the first is returned (the lowest)
*/
std::string shortestPasscode(const char *file) {

    std::ifstream f(file);
    if (!f) {
        std::cerr << "Error opening file";
        return "";
    }
    std::vector<std::string> loginCodes;
    std::string buffer;
    while (f >> buffer) loginCodes.push_back(buffer);

    // Record which digits are used and which appear to the right of each as given by the hints
    bool digits[numDigits] = {};            // true for digits that are used
    bool right[numDigits][numDigits] = {};  // true for [x][y] if x appears before y
    for (auto &c : loginCodes) {
        for (int i = 0; i < c.size(); i++) {
            int digit = c[i] - '0';
            digits[digit] = true;
            for (int j = i+1; j < c.size(); j++)
                right[digit][c[j]-'0'] = true;
        }
    }
    // Recursively add all right neighbors of each digit implied by all hints
    // e.g. if the hints are 12, and 23, 1 has right neighbors 2 and 3
    for (int i = 0; i < numDigits; i++) {
        if (!digits[i]) continue;
        for (int j = 0; j < numDigits; j++) {
            if (!right[i][j]) continue;
            // add j's right neighbors to i
            if (!addRightNeighbors(right, i, j)) {
                std::cout << "Solution contains repeated digits";
                return "";
            }
        }
    }
    // Count how many right neighbors each digit has
    std::vector<std::pair<int, int>> rightCount;    // pair(digit, #neighbors)
    for (int i = 0; i < numDigits; i++) {
        if (digits[i]) {
            int count = 0;
            for (int j = 0; j < numDigits; j++)
                if (right[i][j]) count++;
            rightCount.push_back({i, count});
        }
    }
    // The passcode consists of the digits in descending order of their number of right neighbors
    std::sort(rightCount.begin(), rightCount.end(), [](const auto& i, const auto& j) -> bool { return i.second>j.second; });
    std::string result = "";
    for (const auto& p : rightCount)
        result += std::to_string(p.first);

    return result;
}
