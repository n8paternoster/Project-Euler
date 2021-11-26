#include "pch.h"
#include "76-100/90.h"
#include <array>
#include <algorithm>

int numCubeArrangements() {
	std::array<bool, 10> leftCube = std::array<bool, 10>();
	std::array<bool, 10> rightCube = std::array<bool, 10>();
	for (int i = 0; i < 6; i++)	// initialize left cube to (0, 1, 2, 3, 4, 5)
		leftCube[i] = true;

	constexpr std::pair<int, int> constraints[] = {{0, 1}, {0, 4}, {0, 9}, {1, 6}, {2, 5}, {3, 6}, {4, 9}, {6, 4}, {8, 1}};
	int numArrangements = 0;

	// Check all combinations
	do {
		// Reset right cube
		std::copy(leftCube.begin(), leftCube.end(), rightCube.begin());
		do {
			bool valid = true;
			for (auto c : constraints) {
				int n = c.first;
				int m = c.second;
				if (n == 6 || n == 9 || m == 6 || m == 9) {	
					int nAlt = (n == 6) ? 9 : (n == 9 ? 6 : n);
					int mAlt = (m == 6) ? 9 : (m == 9 ? 6 : m);
					if (!(((leftCube[n]||leftCube[nAlt]) && (rightCube[m]||rightCube[mAlt])) || ((leftCube[m]||leftCube[mAlt]) && (rightCube[n]||rightCube[nAlt])))) {
						valid = false;
						break;
					}
				} else if (!((leftCube[n] && rightCube[m]) || (leftCube[m] && rightCube[n]))) {
					valid = false;
					break;
				}
			}
			if (valid) numArrangements++;
		} while (std::prev_permutation(rightCube.begin(), rightCube.end()));
	} while (std::prev_permutation(leftCube.begin(), leftCube.end()));
	return numArrangements;
}
